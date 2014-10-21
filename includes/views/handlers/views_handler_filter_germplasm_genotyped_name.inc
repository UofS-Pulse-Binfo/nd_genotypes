<?php

/**
 * @file
 * Creates a simple textfield filter with autocomplete.
 */

/**
 * The Germplasm Genotyped views filter handler class (autocomplete textfield).
 */
class views_handler_filter_germplasm_genotyped_name extends views_handler_filter_string {

  /**
   * So that the value form is not inherited.
   */
  function value_form (&$form, &$form_state) {

		$form['value'] = array(
			'#type' => 'textfield',
      '#default_value' => $this->value,
      '#autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_germplasm/name_to_id/Phaseolus/only'
		);

  }

  function exposed_form(&$form, &$form_state) {
    parent::exposed_form($form, $form_state);
    dpm($form, 'exposed form');
  }
}