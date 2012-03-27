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
    
    $form['value'] = array(
      '#type' => 'organism_stock',
      '#default_value' => array(
      	'organism_id' => (isset($form_state['input']['cb_organism_id'])) ? $form_state['input']['cb_organism_id'] : $this->options['cb_organism_id'],
      	'stock_name' => (isset($form_state['input']['cb_stock_name'])) ? $form_state['input']['cb_stock_name'] : $this->options['cb_stock_name']
      ),
    );
    
    // set default based on common name organism argument if present
    if (empty($form['#default_value']['organism_id'])) {
      $common_name_arg_pos = $this->view->argument['common_name']->position;
      $common_name_arg = $this->view->args[ $common_name_arg_pos ];
      if (!empty($common_name_arg)) {
        $common_name = str_replace('-',' ',$common_name_arg);
        $org = tripal_core_chado_select('organism',array('organism_id'), array('common_name'=>$common_name));
        $form['value']['#default_value']['organism'] = $org[0]->organism_id;
      }
    }
  }
  
}