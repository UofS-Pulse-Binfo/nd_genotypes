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
      
      $values[$k]->markedup_parent_residues = wordwrap(implode('',$seq),80," ",TRUE);
    }
  }
}