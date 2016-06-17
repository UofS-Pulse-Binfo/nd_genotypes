<?php
/**
 * @file
 * Provides adniministrative interfaces.
 */

/**
 * Listing Launchpad
 */
function nd_genotypes_admin_landing_page() {
 $output = '';

 return $output;
}

/**
 * Update ND Genotype data (ie: Materialized Views)
 */
function nd_genotypes_admin_sync_mviews($form, $form_state) {

  $form['mviews'] = array(
    '#type' => 'fieldset',
    '#title' => 'Materialized Views',
    '#description' => 'Materialized views are used extensively by this module to
      speed up quering and provide flexibility on how you choose to store your data. <strong>It is important to update these
      materialized views whenever you load in new genotype data.</strong>',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );

  $form['mviews']['storage_method'] = array(
    '#type' => 'select',
    '#title' => 'Storage Method',
    '#description' => 'This is the method you are using to store your genotypic data. You can find more information about the supported storage methods on the ' . l('project wiki.', 'https://github.com/UofS-Pulse-Binfo/nd_genotypes/wiki/How-to-Store-your-Data', array('attributes' => array('target' => '_blank'))),
    '#options' => array(
      'nd_exp' => 'Natural Diversity Experiment',
      'genotype_call' => 'Genotype Call Custom Table',
      'genotype_stock' => 'Genotype Stock table',
      'custom' => 'Custom',
    ),
    '#default_value' => variable_get('nd_genotype_storage_method', 'nd_exp'),
  );

  $form['mviews']['populate_mviews'] = array(
    '#type' => 'submit',
    '#name' => 'sync_mviews',
    '#value' => 'Sync'
  );

/*
  $form['import'] = array(
    '#type' => 'fieldset',
    '#title' => 'Import Genotype Experiments',
    '#description' => '<p>Genotype experiments often contain a lot of data, therefore,
      it is expected that you will load the data using the '
      . l('Tripal Bulk Loader', '') .' rather than trying to enter it manually.
      As such a Bulk Loading template has been provided to aid you in loading
      your marker by genotype matrix data (where columns contain the genotypes
      for a given sample and rows indicate which marker was assayed).</p>
      <p>This template should have been created for you when you installed this
      module but if not or if you would like to revert any changes you\'ve made
      then click the "Update Bulk Loader Template" button below.</p>
      <p>To upload your data, ' .l('create a Bulk Loading Job', '')
      . ' with the template being "ND Genotype: Generic Genotype Experiments". Once you create the job, you
      will need to describe your data file. Each "Constant Set" refers to
      a single column in your data file and you will need to indicate the uniquename
      of the stock those genotypes belong to and the column number (both in
      "Allele Call" and "Unique Name (Generated)" where the first column is "1").
      Once you have enetered all this data (all fields are required) then you
      can submit the job for loading and run it on the command-line like you
      would any other tripal job (using drush trp-run-jobs).</p>',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );

  $form['import']['update_template'] = array(
    '#type' => 'submit',
    '#name' => 'update_template',
    '#value' => 'Update Bulk Loader Template'
  );
*/

  return $form;
}

/**
 * Update ND Genotype data (ie: Materialized Views)
 */
function nd_genotypes_admin_sync_mviews_submit($form, $form_state) {

  if ($form_state['triggering_element']['#name'] == 'sync_mviews') {

    // Save the storage method for use later.
    variable_set('nd_genotype_storage_method', $form_state['values']['storage_method']);

    // If they chose a custom way to store their data then we can't help them :-(.
    if ($form_state['values']['storage_method'] == 'custom') {
      drupal_set_message('Unfortunatly we cannot update the materialized view if you saved your data differently then the supported methods. We suggest you cureate your own script to update the materialize view as you will still get all the functionality provided by this module if you do.', 'warning');
    }
    // Create a Tripal job for syncing the materialized view.
    else {

      $name = '';
      $function = '';
      switch($form_state['values']['storage_method']) {
        case 'nd_exp':
          $name = "Update ND Materialized View. Method: Natural Diversity Experiment tables";
          $function = 'nd_genotypes_update_mview_for_nd_exp';
          break;
        case 'genotype_call':
          $name = "Update ND Materialized View. Method: Genotype Call Custom table";
          $function = 'nd_genotypes_update_mview_for_genotype_call';
          break;
        case 'genotype_call':
          $name = "Update ND Materialized View. Method: Genotype Stock table";
          $function = 'nd_genotypes_update_mview_for_genotype_stock';
          break;
      }
      global $user;
      tripal_add_job($name, 'nd_genotypes', $function, array(), $user->uid);
    }
  }

}
