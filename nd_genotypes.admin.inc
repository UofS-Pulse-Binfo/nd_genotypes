<?php

/**
 * Basic Description of the Module
 */
function nd_genotypes_admin_description () {
	$output = '';
	
	$output .= "<h2>Quick Links</h2>";
	$output .= "<ul>";
	$output .= "<li>".l('configuration','Configuration')."</li>";
	$output .= "</ul>";
	
	return $output;
}

/**
 * Configuration Form
 *
 * @param $form_state
 *   The current state of the form
 *
 * @return
 *   An array describing the configuration form
 */
function nd_genotypes_configuration_form ($form_state = NULL) {
	$form = array();
	
	$form['loaders'] = array(
		'#type' => 'fieldset',
		'#title' => t('Bulk Loader Templates'),
	);
	
	$form['loaders']['create_template'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Add Templates'),
		'#description' => t('If checked, will add a Generic genotype experiment Tripal Bulk Loader Template which can be used for any Tripal Bulk Loading Job'),
		'#options' => array(
			'generic_experiment' => 'ND Genotype: Generic Genotype Experiments'
		),
	);

	$form['loaders']['revert_template'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Revert Templates'),
		'#description' => t('If checked, revert the following templates to their original form. ALL CHANGES WILL BE LOST.'),
		'#options' => array(
			'generic_experiment' => 'ND Genotype: Generic Genotype Experiments'
		),
	);
	
	$form['save'] = array(
		'#type' => 'submit',
		'#value' => t('Save Configuration'),
		'#name' => 'save'
	);
	
	return $form;
}

/**
 * Configuration Form Submit
 *
 * @param $form
 *   Te form that was submitted
 * @param $form_state
 *   The current state of the form
 */
function nd_genotypes_configuration_form_submit ($form, &$form_state) {
	
	if ($form_state['clicked_button']['#name'] == 'save') {
		
		// Create any Templates selected to be created
		if ($form_state['values']['create_template']['generic_experiment']) {
			nd_genotypes_add_generic_experiment_bulk_loader();
		}
		
		// Revert any Templates selected to be Reverted
		if ($form_state['values']['revert_template']['generic_experiment']) {
			nd_genotypes_add_generic_experiment_bulk_loader(TRUE);
		}
		
	}
}