<?php

/**
 * Basic Description of the Module
 */
function nd_genotypes_admin_launchpad () {
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

function nd_genotypes_mviews_update_form ($form_state = NULL) {
  $form = array();

  $form['msg'] = array(
    '#type' => 'item',
    '#value' => 'This is the preferred method to update materialized views used by the
      Natural Diversity Genotypes module. This will ensure that any additional
      materialized views or queries that need to be are run when a given materialized view
      is updated.'
  );

  $form['mviews'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Materialized Views to Update',
    '#options' => array(
      'nd_genotype_experiment' => 'ND Genotype Experiment: This is the main view needed to display genotypes associated with a given stock for a list of features.',
      'nd_genotype_experiment_summary' => 'ND Genotype Experiment Summary: This view is used to count the number of polymorphic/monomorphic markers between two stocks.'
    ),
  );

  $form['update'] = array(
    '#type' => 'submit',
    '#value' => 'Update Selected',
  );

  $form['update_all'] = array(
    '#type' => 'submit',
    '#value' => 'Update All',
  );

  return $form;
}

function nd_genotypes_mviews_update_form_submit ($form, $form_state) {
  global $user;

  foreach ($form_state['values']['mviews'] as $mview_name => $checkbox_value) {
    // update the materialized view if either:
    // 1) it was checked
    // 2) Update All was clicked
    if ($checkbox_value OR ($form_state['values']['op'] == $form_state['values']['update_all'])) {
      tripal_add_job(
        'Update '.$mview_name,
        'nd_genotypes',
        'nd_genotypes_register_'.$mview_name.'_mview',
        array(),
        $user->uid
      );
    }
  }
}