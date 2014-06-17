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

  function init (&$view, &$options) {
    parent::init($view, $options);

    if (isset($_GET['add_germplasm_genotype']['add_stock'])) {
      if (!empty($_GET['add_germplasm_genotype']['add_stock']['stock_name']) AND !empty($_GET['add_germplasm_genotype']['add_stock']['organism_id'])) {
        $add_stock = $_GET['add_germplasm_genotype']['add_stock'];
  
        // Find the maximum key
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
  
        $stock = tripal_get_stock(array('name' => $add_stock['stock_name'], 'organism_id' => $add_stock['organism_id']));
  
        if (is_object($stock)) {
          if (tripal_germplasm_has_nd_genotypes($stock->stock_id)) {
            $values = $_GET['add_germplasm_genotype'];
            $values['existing'][] = array(
              'stock_name' => $add_stock['stock_name'],
              'organism_id' => $add_stock['organism_id'],
              'stock_id' => $stock->stock_id,
            );
            unset($values['add_stock']);
  
            $_GET['add_germplasm_genotype'] = $values;
          }
          else {
            $msg = 'That stock does not have any genotypes associated with it. Keep in mind that
              you currently have to enter the full name of the DNA stock that was assayed. For example,
              "964a-46 LAM Plate1 Extraction" instead of just "964a-46". For a list of stocks assayed
              click on the "Germplasm Genotyped" Link in the menu under the genotype search.';
            drupal_set_message($msg,'error');
            tripal_report_error(
              'nd_genotype',
              TRIPAL_WARNING,
              $msg
            );
          }
        }

      }
      // If they entered a stock name but no organism then iform them this is So Not Okay ;)
      elseif (!empty($_GET['add_germplasm_genotype']['add_stock']['stock_name'])) {
        drupal_set_message('You must enter the source species when adding genotypes.', 'error');
      }
      // If they didn't supply either stock name or organism_id then maybe they just want to remove things
      else {
        // remove empty add_stock
        unset($_GET['add_germplasm_genotype']['add_stock']);

        // Then check to see if any need to be removed
        if (isset($_GET['add_germplasm_genotype']['existing'])) {
          foreach($_GET['add_germplasm_genotype']['existing'] as $k => $spec) {
            if (isset($spec['remove'])) {
              if ($spec['remove']) {
                unset($_GET['add_germplasm_genotype']['existing'][$k]);
              }
            }
          }
        }


      }
    } //end of if add_germplasm_genotype is set

    if (isset($_GET['add_germplasm_genotype'])) {
  		$this->value = $_GET['add_germplasm_genotype'];
    }

		// Now add fields
		// Add them here rather then pre_query so they get added to exports
		// Check if already set since init is run multiple times
		if (empty($this->view->germplasm_genotype_fields_added)) {

      // every-time this is run each previously added as well as the newly added germplasm genotype fields
      // need to be added to the view. This is due to the state of the view not being saved when the Apply button is clicked.
      // Also ensures if you bookmark it you'll still have your added fields
      if (!$this->value) { $this->value = array(); }
      $this->view->germplasm_genotype_fields_added = 0;
      if (!isset($this->value['existing'])) { $this->value['existing'] = array(); }
      $this->view->germplasm_genotype_specs = $this->value['existing'];
      foreach ($this->value['existing'] as $i => $spec) {

        // Add Field //////////////////////
        $max_key = 0;
        $max_position = 0;
        if (!isset($existing_field)) {
          foreach ($this->view->field as $key => $field) {
            //determine the max key and store an existing germplasm_genotype field for cloning
            if (preg_match('/germplasm_genotype_{0,1}(\d*)/',$key,$matches)) {
              $existing_field = $field;
              if (!empty($matches[1]) && $matches[1] > $max_key) {
                $max_key = $matches[1];
              }
            }
          }
        }

        if ($existing_field) {
          $this->view->germplasm_genotype_fields_added++;
          $alias = 'germplasm_genotype_'. ($max_key+1+$i);

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
          $new_field->options['exclude'] = 0;

          if (isset($new_field->options['element_class'])) {
            $new_field->options['element_class'] = str_replace('[germplasm_genotype]','['.$alias.']', $new_field->options['element_class']);
          }

          $new_field->stock_name = array($spec['stock_id'] => $spec['stock_name']);
          $new_field->organism_id = array($spec['stock_id'] => $spec['organism_id']);
          $new_field->stock_id = array($spec['stock_id'] => $spec['stock_id']);
          $new_field->original_stock_id = $spec['stock_id'];
          $new_field->original_stock_name = $spec['stock_name'];

          // add field to current display
          $this->view->field[ $alias ] = $new_field;

          //also add to default and export view displays
          foreach ($this->view->display as $display_name => $display) {
            $this->view->display[$display_name]->display_options['fields'][ $alias ] = (array) $new_field;
            $this->view->display[$display_name]->handler->options['fields'][ $alias ] = $new_field->options;
          }
        }
      }
    }
  }

  // Add the fields to the view before the query is run
  function pre_query() {
    parent::pre_query();

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
  function value_form (&$form, &$form_state) {

    $form_element_name = $this->options['id'];

    // GET OPTIONS BASED ON ORGANISM ARGUEMENTS
    if (!empty($this->view->argument)) {
      $org_values = array();
      foreach ($this->view->argument as $arg) {
        if ($arg->table == 'organism') {
          $org_values[$arg->field] = $this->view->args[ $arg->position ];
        }
      }
      if (!empty($org_values)) {
        $options = array('order_by' => array(
          'organism_id' => 'ASC'
        ));
        $organism = chado_select_record('organism',array('organism_id', 'genus', 'species'),$org_values,$options);
        if (is_array($organism)) {
          $organism_options[0] = ' ';
          foreach ($organism as $o) {
            $organism_options[$o->organism_id] = $o->genus .' '. $o->species;
          }
        }
      }
    }

		$form['value'] = array(
			'#type' => 'multiple_organism_stock',
      '#organism_title' => 'Source Species',
      '#default_value' => $this->value,
      '#organism_weight' => 10,
      '#added_description' => 'The following stocks have already had their genotypes added to the listing below:'
		);
		if (isset($organism_options) AND sizeof($organism_options) > 1) {
      $form['value']['#organism_options'] = $organism_options;
    }
  }

}
