<?php

/**
 * This filter adds germplasm genotype fields to the view as described in the path
 *
 * It must be used in conjunction with the following code in the header of the view. This code
 * adds a popup to the view that allows the user to define the new field parameters.
 * @code
    <?php
      $view = view_get_current_view();
      print germplasm_manage_addremove_genotype_column_button($view);
    ?>
 * @endcode
 *
 * This filter is needed to add the fields before the query is run. You cannot add the fields
 * in the submit of the above added form because at that point the view is already rendered.
 */
class views_handler_filter_add_germplasm_genotype_by_stock_field_filter extends views_handler_filter_germplasm_genotype_by_stock {

  function init (&$view, $options) {
    parent::init($view, $options);
    			
    if (isset($_GET['add_germplasm_genotype']['add_stock']) && $_GET['add_germplasm_genotype']['add_stock']['stock_name']) {
			$add_stock = $_GET['add_germplasm_genotype']['add_stock'];
			
			$max_key = 'N';
			foreach ($_GET['add_germplasm_genotype'] as $k => $v) {
				if (preg_match('/^\d+$/',$k)) {
					if ($max_key == 'N') {
						$max_key = $k;
					} elseif ($max_key < $k) {
						$max_key = $k;
					}
				}
			}
			if ($max_key === 'N') {
				$i = 0;
			} else {
				$i = intval($max_key) + 1;
			}
			
			$stock = tripal_stock_get_stock_by_name_identifier($add_stock['stock_name'], $add_stock['organism_id']);
		
			if ($stock[0]->stock->stock_id) {
				$values = $_GET['add_germplasm_genotype'];
				$values[$i]['stock_name'] = $add_stock['stock_name'];
				$values[$i]['organism_id'] = $add_stock['organism_id'];
				$values[$i]['stock_id'] = $stock[0]->stock->stock_id;
				unset($values['add_stock']);
				
				$_GET['add_germplasm_genotype'] = $values;
			}

		} elseif (isset($_GET['add_germplasm_genotype'])) {
			// remove empty add_stock
			unset($_GET['add_germplasm_genotype']['add_stock']);
			
			// Then check to see if any need to be removed
			foreach($_GET['add_germplasm_genotype'] as $k => $spec) {
				if ($spec['remove']) {
					unset($_GET['add_germplasm_genotype'][$k]);
				}
			}
			
			
		}
		
		$this->value = $_GET['add_germplasm_genotype'];
    
  }

  // Add the fields to the view before the query is run
  function pre_query() {
    parent::pre_query();

    // every-time this is run each previously added as well as the newly added germplasm genotype fields
    // need to be added to the view. This is due to the state of the view not being saved when the Apply button is clicked.
    // Also ensures if you bookmark it you'll still have your added fields
    if (!$this->value) { $this->value = array(); }
    foreach ($this->value as $i => $spec) {
      
      // Add Field //////////////////////
      $max_key = 0;
      $max_position = 0;
      foreach ($this->view->field as $key => $field) {
        //determine the max key and store an existing germplasm_genotype field for cloning
        if (preg_match('/germplasm_genotype_{0,1}(\d*)/',$key,$matches)) {
          $existing_field = $field;
          if (!empty($matches[1]) && $matches[1] > $max_key) {
            $max_key = $matches[1];
          }
        }
        
        // determine the last position already containing a field
        if ($field->position > $max_position) {
          $max_position = $field->position;
        }
      }

      if ($existing_field) {
        $alias = 'germplasm_genotype_'. ($max_key+1);
               
        // Add the new field to the view
        $new_field = new views_handler_field_germplasm_genotype();
        $new_field->field_alias = 'feature_id';
        $new_field->aliases['feature_id'] = 'feature_id';
        $new_field->options = $existing_field->options;
        $new_field->definition = $existing_field->definition;
        $new_field->additional_fields = array();
        $new_field->view = $existing_field->view;
        $new_field->table = 'genotype';
        $new_field->field = 'germplasm_genotype';
        $new_field->real_field = 'germplasm_genotype';
        $new_field->query = $existing_field->query;
        
        $new_field->aliases['genotypes'] = $alias;        
        $new_field->options['label'] = 'New Germplasm Genotype';
        $new_field->options['id'] = $alias;
        $new_field->options['use_filter'] = '<NONE>';
        $new_field->options['germplasm']['stock_name'] = $spec['stock_name'];
        $new_field->options['germplasm']['organism_id'] = $spec['organism_id'];
        $new_field->options['exclude'] = FALSE;
        $new_field->stock_name = array($spec['stock_id'] => $spec['stock_name']);
        $new_field->organism_id = array($spec['stock_id'] => $spec['organism_id']);
        $new_field->stock_id = array($spec['stock_id'] => $spec['stock_id']);
        $new_field->original_stock_id = $spec['stock_id'];
        $new_field->original_stock_name = $spec['stock_name'];
        $new_field->position = $max_position+1;

        $this->view->field[ $alias ] = $new_field;
      }
    }
    
  }

  // This is needed to ensure query is not inherited
  // parent filter adds feature_id IS NULL when value is empty
  function query () { }
  
  // Modify the options form
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    
    unset($form['operator'], $form['case']);
  }
  
  /**
   * So that the value form is not inherited
   */
  function value_form (&$form, $form_state) {
		$form['value'] = array(
			'#type' => 'multiple_organism_stock',
			'#default_value' => $this->value
		);
  }

}