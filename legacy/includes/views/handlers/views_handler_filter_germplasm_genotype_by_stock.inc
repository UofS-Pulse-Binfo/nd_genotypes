<?php

/**
 * This filter changes the defaults of a germplasm genotype field
 *
 * NOTE: This field does nothing on it's own. To use, set the "Use Filter" option in the germplasm
 * genotype field to point to an instance of this filter. Then the value of this filter will change
 * default of the field and thus the data rendered. This is only useful as an exposed filter
 */
class views_handler_filter_germplasm_genotype_by_stock extends views_handler_filter_string {

  // This is needed to ensure that a field is not added to the query based on this filter
  function query () {

  }

  // Defines the exposed form
  function value_form (&$form, &$form_state) {
    parent::value_form($form, $form_state);

    $form_element_name = $this->options['id'];

    // SET DEFAULT
    $default = array('organism_id' => 0, 'stock_name' => '');
    if (!empty($form_state['input'][$form_element_name]['stock_name'])) {
      $default = $form_state['input'][$form_element_name];
    }
    elseif (!empty($this->value['stock_name'])) {
      $default = $this->value;
    }

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
          $organism_options['<ANY>'] = '<Any>';
          foreach ($organism as $o) {
            $organism_options[$o->organism_id] = $o->genus .' '. $o->species;
          }
        }
      }
    }

    $form['value'] = array(
      '#type' => 'organism_stock',
      '#organism_title' => 'Source Species',
      '#default_value' => $default,
      '#organism_weight' => 10
    );
    if (isset($organism_options) AND sizeof($organism_options) > 1) {
      $form['value']['#organism_options'] = $organism_options;
    }

  }

}