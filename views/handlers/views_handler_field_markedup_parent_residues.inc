<?php

class views_handler_field_markedup_parent_residues extends views_handler_field {

  /**
   * Iinitialize the handler
   */
  function init (&$view, $options) {
    parent::init($view, $options);
    
    //Add aliases so content added at pre_render stage is rendered in this field
    $this->alias = 'markedup_parent_residues';
    $this->aliases['markedup_parent_residues'] = 'markedup_parent_residues';
    $this->field_alias = $this->aliases['markedup_parent_residues'];   

    if (!isset($this->IUPAC)) {
      $this->IUPAC = array(
        
        'AG' => 'R',
        'GA' => 'R',
        
        'CT' => 'Y',
        'TC' => 'Y',
        
        'GC' => 'S',
        'CG' => 'S',
        
        'AT' => 'W',
        'TA' => 'W',
        
        'GT' => 'K',
        'TG' => 'K',
    
        'AC' => 'M',
        'CA' => 'M',
        
        'CGT' => 'B',
        'GCT' => 'B',
        'TGC' => 'B',
        'TCG' => 'B',
        
        'AGT' => 'D',
        'GAT' => 'D',
        'TGA' => 'D',
        'TAG' => 'D',
        
        'ACT' => 'H',
        'CAT' => 'H',
        'TCA' => 'H',
        'TAC' => 'H',
        
        'ACG' => 'V',
        'CAG' => 'V',
        'GAC' => 'V',
        'GCA' => 'V',
      );
    }    
  }
  
  /**
   * Add Fields and Joins to the Query
   */
  function query() {
    
    // Add join to featureloc table
    $this->query->ensure_table('featureloc');
			
    // Add fields: featureloc.fmin, featureloc.fmax
    $this->aliases['fmin'] = $this->query->add_field('featureloc','fmin');
    $this->aliases['fmax'] = $this->query->add_field('featureloc','fmax');
    
    // Add join to feature (parent) table
    $table_alias = 'feature_parent';
    $def = array();
    $def['table'] = 'feature';
    $def['field'] = 'feature_id';
    $def['left_table'] = 'featureloc';
    $def['left_field'] = 'srcfeature_id';

    $join = new views_join();
    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;
    $this->query->add_relationship($table_alias, $join, $def['table']);
    
    // Add field: feature.residues
    $this->aliases['parent_residues'] = $this->query->add_field('feature_parent','residues');
    
    // Add join to feature_genotype/genotype tables in such a way that the 1 feature to many genotypes is
    // condensed down to a single genotypes field (ie: A/T)
    $table_alias = 'feature_genotypes';
    $def = array();
    $def['table'] = "(SELECT fg.feature_id, array_to_string(array_agg(g.description),'/') as genotype FROM genotype g LEFT JOIN feature_genotype fg ON fg.genotype_id=g.genotype_id GROUP BY fg.feature_id)";
    $def['field'] = 'feature_id';
    $def['left_table'] = 'feature';
    $def['left_field'] = 'feature_id';

    $join = new views_join();
    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;
    $this->query->add_relationship($table_alias, $join, $def['table']);
    
    // Add field: nd_genotype_experiment.genotype_description
    // aggregate this into an array since feature => genotype is a 1:many relationship
    $this->aliases['genotypes'] = $this->query->add_field($table_alias,'genotype');
    
    // Add join to list of adjacent loci with genotypes
    $table_alias = 'adjacent_feature_genotypes';
    $def = array();
    $def['table'] = "(SELECT am.srcfeature_id, array_agg(am.fmin||'::'||am.fmax||'::'||am.genotype) as adjacent_markers 
                    FROM (
                      SELECT fl.srcfeature_id, fl.feature_id, fl.fmin, fl.fmax, array_to_string(array_agg(g.description),'/') as genotype 
                      FROM featureloc fl 
                      LEFT JOIN feature_genotype fg ON fg.feature_id=fl.feature_id 
                      LEFT JOIN genotype g ON fg.genotype_id=g.genotype_id 
                      WHERE g.genotype_id IS NOT NULL 
                      GROUP BY fl.srcfeature_id, fl.feature_id, fl.fmin, fl.fmax 
                      ORDER BY fl.fmin) am 
                    GROUP BY am.srcfeature_id)";
    $def['field'] = 'srcfeature_id';
    $def['left_table'] = 'featureloc';
    $def['left_field'] = 'srcfeature_id';

    $join = new views_join();
    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;
    $this->query->add_relationship($table_alias, $join, $def['table']);

    // Add field: adjacent_feature_genotypes.adjacent_markers
    $this->aliases['adjacent_markers'] = $this->query->add_field($table_alias,'adjacent_markers');
  }
  
  /**
   * Choose bits of the query and process into the marked-up parent sequence
   */
  function pre_render (&$values) {
    
    foreach ($values as $k => $v) {
      $seq = str_split($v->{$this->aliases['parent_residues']});
      if (empty($seq[0])) { continue; }
      
      $start = $v->{$this->aliases['fmin']}-1;
      $end = $v->{$this->aliases['fmax']}-2;
      
      // Add in A/T SNP notation if present
      if ($v->{$this->aliases['genotypes']}) {
        if ($start == $end) {
          $seq[$start] = $v->{$this->aliases['genotypes']};
        }
      }
      
      // Add in [] markup at SNP position
      $seq[$start] = '['.$seq[$start];
      $seq[$end] = $seq[$end].']';
      
      // Now deal with adjacent markers
      $v->{$this->aliases['adjacent_markers']} = $this->split_adjacent_markers($v->{$this->aliases['adjacent_markers']});
      foreach ($v->{$this->aliases['adjacent_markers']} as $am) {
        $seq[$am['fmin']] = $am['IUPAC'];
      }
      
      $values[$k]->markedup_parent_residues = wordwrap(implode('',$seq),80," ",TRUE);
    }

  }
  
  function split_adjacent_markers ($v) {
    
    // first split apart outer postgreSQL array
    if(preg_match('/^{(.*)}$/',$v, $matches)) {
      $v = str_getcsv($matches[1]);
      
      // then split apart individual records
      foreach ($v as $vk => $vv) {
        $t = explode('::',$vv);
        
        // then split apart genotypes
        $t[2] = explode('/',$t[2]);
        
        // then add back to $v
        $new_v[$t[0]] = array(
          'fmin' => $t[0],
          'fmax' => $t[1],
          'genotypes' => $t[2],
          'IUPAC' => $this->IUPAC[implode('',$t[2])]
        );
      }
    } else {
      $new_v = array();
    }
    
    ksort($new_v);
    return $new_v;
  }
}