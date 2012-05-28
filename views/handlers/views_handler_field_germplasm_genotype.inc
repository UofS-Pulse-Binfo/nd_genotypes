<?php

/**
 * Defines a field which displays the genotypes of a specified germplasm for the features listed
 * in the view
 *
 * NOTE: This field does not add to the main views query but instead executes it's own query
 * on a materialized view (kp_stock_genotype_counts)
 */
class views_handler_field_germplasm_genotype extends views_handler_field_prerender_list {

  function init(&$view, $options) {
    parent::init($view, $options);
    
    $this->aliases['feature_id'] = 'feature_id';
    $this->field_alias = $this->aliases['feature_id'];   

    // Set aliases
    $this->aliases['genotypes'] = $this->options['id'];
    $this->aliases['feature_id'] = 'feature_id';
    $this->field_alias = $this->aliases['feature_id'];

    
    // Determine stock name and organism_id and stock_id
    $filter_identifier = $this->view->display['default']->display_options['filters'][ $this->options['use_filter'] ]['expose']['identifier'];
    $filter_value = $_GET[ $filter_identifier ];
    if (!empty($filter_value['stock_name']) ) {
      $this->stock_name = $filter_value['stock_name'];
      $this->organism_id = $filter_value['organism_id'];
    } elseif (!empty($this->options['germplasm']['stock_name'])) {
      $this->stock_name = $this->options['germplasm']['stock_name'];
      $this->organism_id = $this->options['germplasm']['organism_id'];
    }
    
    if ($this->stock_name) {
      $stock = tripal_stock_get_stock_by_name_identifier($this->stock_name, $this->organism_id);
      if (!empty($stock[0]->stock->stock_id)) {
        $this->stock_id[$stock[0]->stock->stock_id] = $stock[0]->stock->stock_id;
        $this->stock_name[$stock[0]->stock->stock_id] = $this->stock_name;
        $this->organism_id[$stock[0]->stock->stock_id] = $this->organism_id;
        $this->original_stock_id = $stock[0]->stock->stock_id;
        $this->original_stock_name = $this->stock_name;
      } else {
        drupal_set_message('There are no stocks with the name, "'.$this->stock_name.'", associated with that organism.', 'error');
      }
    }

  }

  // Set the label and ensure the genotypes exist (if that option is checked)
  function query() {
    
    //Set label to be stock_name
    if (!empty($this->original_stock_name)) {
      foreach ($this->view->display as $display) {
        $display->handler->options['fields'][ $this->aliases['genotypes'] ]['label'] = $this->original_stock_name;
      }
      $this->options['label'] = $this->original_stock_name;
    }
    
    // Add additional stocks which are selections of the current stock
    $selections = tripal_core_chado_select('stock_relationship',array('subject_id'),array('object_id'=>$this->original_stock_id, 'type_id'=> array('name'=>'is_selection_of')));
		foreach ($selections as $s) {
			$stock = tripal_core_chado_select('stock',array('stock_id','organism_id','name'),array('stock_id'=>$s->subject_id));
			if ($stock[0]->organism_id) {
				$this->stock_id[ $stock[0]->stock_id ] = $stock[0]->stock_id;
				$this->stock_name[ $stock[0]->stock_id ] = $stock[0]->name;
				$this->organism_id[ $stock[0]->stock_id ] = $stock[0]->organism_id;
			}
		}
		
		// Add to query
		if (sizeof($this->stock_id) > 0) {
			// Make our own join
			// (don't just ensure_my_table b/c this way we can limit to only showing genotype_description)
			$table_alias = 'nd_genotype_experiment_stock_'.$this->original_stock_id;
			$def = array();
			if (sizeof($this->stock_id) == 1) {
				$def['table'] = "(SELECT feature_id, array_agg(genotype_description) as genotype_description, array_agg(genotype_id) as genotype_id, ':'||array_to_string(array_agg(polymorphic_with),':')||':' as polymorphic_with FROM nd_genotype_experiment WHERE stock_id=".reset($this->stock_id).' GROUP BY feature_id)';
			} else {
				$def['table'] = "(SELECT feature_id, array_agg(genotype_description) as genotype_description, array_agg(genotype_id) as genotype_id, ':'||array_to_string(array_agg(polymorphic_with),':')||':' as polymorphic_with FROM nd_genotype_experiment WHERE stock_id IN (".implode(',',$this->stock_id).') GROUP BY feature_id)';
			}
			$def['field'] = 'feature_id';
			$def['left_table'] = 'feature';
			$def['left_field'] = 'feature_id';
	
			$join = new views_join();
			$join->definition = $def;
			$join->construct();
			$join->adjusted = TRUE;
	
			$this->query->add_relationship($table_alias, $join, $def['table']);
			
			// genotype_description field
			$this->alias = $this->query->add_field($table_alias,'genotype_description', 'stock'.$this->original_stock_id.'_allele');
			
			// genotype_id field
			$this->genotype_id_field_alias = $this->query->add_field($table_alias,'genotype_id', 'stock'.$this->original_stock_id.'_genotype_id');				

			$this->added_to_query = TRUE;
		}
  }
  
  // Adds a number of fields to the option form (displayed to admin to configure field)
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // specify the germplasm to show genotypes for
    $form['germplasm'] = array(
      '#type' => 'organism_stock',
      '#title' => 'Default Germplasm',
      '#description' => 'Enter the default germplasm genotypes should be shown for.',
      '#default_value' => $this->options['germplasm'],
    );
    
    // set the germplasm_genotype filter that can change the above default
    $options['<NONE>'] = 'None';
    if (isset($this->view->display['default']->display_options['filters'])) {
      foreach ($this->view->display['default']->display_options['filters'] as $name => $filter) {
        if ($filter['field'] == 'germplasm_genotype') {
          if (isset($filter['expose'])) {
            $options[$name] = $filter['expose']['label'];
          } else {
            $options[$name] = $name;
          }
        }
      }
    }
    $form['use_filter'] = array(
      '#type' => 'select',
      '#title' => t('Use Germplasm Genotype Filter'),
      '#description' => 'Select a filter which can be used to change the default germplasm set above.',
      '#options' => $options,
      '#default_value' => $this->options['use_filter'],
    );

    // ensure that only features with genotypes associated with the current germplasm are listed
    $form['has_genotype'] = array(
      '#type' => 'checkbox',
      '#title' => 'Require genotype exists for this stock',
      '#default_value' => (isset($this->options['has_genotype'])) ? isset($this->options['has_genotype']) : 1,
    );
    
    // pulls in genotypes from selections of the current stock
    // ie: from stocks connected to the current stock (object) via is_selection_of (type) relationship
    $form['get_selections'] = array(
      '#type' => 'checkbox',
      '#title' => 'Display genotypes from selections of the current stock',
      '#description' => 'Adds genotypes from any stock that is the subject in a <subject> is_selection_of <current stock> relationship.',
      '#default_value' => (isset($this->options['get_selections'])) ? isset($this->options['get_selections']) : 1,
    );
    
    // set the way the genotype is rendered
    // useful when there is 2+ genotypes per marker per germplasm combination
    $form['render_type'] = array(
      '#type' => 'radios',
      '#title' => 'Render Multiple Genotypes as',
      '#options' => array(
        'list' => 'List multiple Genotypes separated as indicated by display type (ie: A, T, A)',
        'consensus' => 'Display the Consensus Genotype (ie: A)',
        'ratio' => 'Display the ratio of genotypes (ie: 66% A : 33% T)',
        'both' => 'Display the consensus followed by the ratio of genotypes (ie: A (66% A : 33% T))'
      ),
      '#default_value' => ($this->options['render_type']) ? $this->options['render_type'] : 'both'
    );
    
    // If checked will colour the bases according to the sanger sequencing standards
    // (ie: C: blue, G: green, A: black, T: red)
    // Uses a CSS rule
    $form['color_bases'] = array(
      '#type' => 'checkbox',
      '#title' => 'Color bases',
      '#default_value' => ($this->options['color_bases']) ? $this->options['color_bases'] : TRUE
    );
    
  }

  // Add genotypes to $view->result (results of the query) and $this->items (used in rendering)
  function pre_render($values) {  
    
    if ($this->added_to_query) {
			foreach ($values as $k => $v ) {
				$values[$k]->{$this->alias} = $this->chado_wrapper_split_array_agg_results($v->{$this->alias});
				$values[$k]->{$this->genotype_id_field_alias} = $this->chado_wrapper_split_array_agg_results($v->{$this->genotype_id_field_alias});
	
				$num_genotypes = sizeof($values[$k]->{$this->alias});
				
				foreach ($values[$k]->{$this->alias} as $sk => $allele) {
					$genotype_id = $values[$k]->{$this->alias}[$sk];
					$this->items[$v->feature_id][$genotype_id]['feature_id'] = $v->feature_id;
					$this->items[$v->feature_id][$genotype_id]['genotype_id'] = $genotype_id;
					$this->items[$v->feature_id][$genotype_id]['genotype'] = $allele;          
					$this->items[$v->feature_id][$genotype_id]['percent_per_genotype_per_stock'] = 1 / $num_genotypes;
	
					$this->counts[ $v->feature_id ][ $genotype_id ]++;
				}
			}
			
			// Keep track of stocks with genotypes so we can alert the user where the genotypes are comming from
			$stocks_with_genotypes = array();
			
			// if there are any stocks with genotypes
			if (sizeof($stocks_with_genotypes)) {
				// and either there are 2+ stocks with genotypes or the only stock with genotypes isn't the original one
				if (!(sizeof($stocks_with_genotypes)==1 && isset($stocks_with_genotypes[$this->original_stock_id]))) {
					drupal_set_message('Genotypes displayed for '.$this->original_stock_name.' include those from the following selections: '.implode(', ',$stocks_with_genotypes).'.', 'warning');
				}
			}
		}
  }

  /**
   * Render all items in this field together.
   *
   * When using advanced render, each possible item in the list is rendered
   * individually. Then the items are all pasted together.
   */
  function render_items($items) {

    if (empty($items)) {
      return '';
    }
    
    if ((sizeof($items) == 1) OR ($this->options['render_type'] == 'list')) {
      $rendered_items = parent::render_items($items);
      
      // colour all bases
      if ($this->options['color_bases']) {
        $rendered_items = preg_replace('/T/', "<span class='genotype' id='T'>T</span>", $rendered_items);
        $rendered_items = preg_replace('/A/', "<span class='genotype' id='A'>A</span>", $rendered_items);
        $rendered_items = preg_replace('/G/', "<span class='genotype' id='G'>G</span>", $rendered_items);
        $rendered_items = preg_replace('/C/', "<span class='genotype' id='C'>C</span>", $rendered_items);
      }
      
    } else {
      
      $max_percent = 0;
      $consensus = '';
      $parts = array();
      foreach ($items as $i) {
        preg_match('/^(\d+\.*\d*)% (.*)$/',$i, $matches);
        if ($max_percent < $matches[1]) {
          $max_percent = $matches[1];
          if (!preg_match('/\?/',$matches[2])) {
            $consensus = $matches[2];
          }
        }
      }
      // just in case all of them are questionable
      // assign the last one so it's not blank
      if (!$consensus) {
        $consensus = $matches[2];
      }
      
      if ($this->options['color_bases']) {
        $consensus = preg_replace('/T/', "<span class='genotype' id='T'>T</span>", $consensus);
        $consensus = preg_replace('/A/', "<span class='genotype' id='A'>A</span>", $consensus);
        $consensus = preg_replace('/G/', "<span class='genotype' id='G'>G</span>", $consensus);
        $consensus = preg_replace('/C/', "<span class='genotype' id='C'>C</span>", $consensus);
      }
      
      switch($this->options['render_type']) {
        case 'consensus':
          $rendered_items = $consensus;
          break;
        case 'ratio':
          $rendered_items = implode(' : ',$items);
          break;
        case 'both':
          $rendered_items = $consensus . ' (' . implode(' : ',$items) . ')';
          break;
        default:
          $rendered_items = $consensus . ' (' . implode(' : ',$items) . ')';
          break;
      }

    }

    return $rendered_items;
  }
  
  // render a single item
  function render_item($count, &$item) {
    $output = '';
    
    // Set the percent of the item
    // needed to be done here so we had the full count of stocks with genotypes
    if (sizeof($this->stock_ids) == 1) {
      $item['percent'] = $item['percent_per_genotype_per_stock'];
    } else {
      $percent = ($item['percent_per_genotype_per_stock'] / sizeof($this->counts[ $item['feature_id'] ]) ) * 100;
      $item['percent'] = $percent;
    }
    
    // render the items
    if ($item['percent'] == '100') {
      $output = $item['genotype'];
    } else {
      $output = $item['percent'] . '% ' . $item['genotype'];
    }
    
    return $output;
  }

  /**
   * Splits an SQL array of results in a single field
   * into a php array
   *
   * @param $field
   *   An SQL array (ie: {"",^(.*)$,646,tripal_analysis_blast} )
   * @return
   *   A PHP version of the SQL array (ie: array('','^(.*)$','646','tripal_analysis_blast') )
   */
  function chado_wrapper_split_array_agg_results($field) {
  
  	if (is_array($field)) {
  		return $field;
  	}
  	
    if(preg_match('/^{(.*)}$/',$field, $matches)) {
      return str_getcsv($matches[1]);
    } else {
      return array();
    }
    
    return $values;
  }
}