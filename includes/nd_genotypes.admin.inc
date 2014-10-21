<?php

/**
 * @file
 * Administrative forms
 */


/**
 * Listing Launchpad
 */
function nd_genotypes_admin_landing_page() {
 $output = '';

 // Get the View Embed Code
 // This function will return FALSE if your view is not enabled
 $view_code = views_embed_view('nd_genotype_experiments_admin', 'page');
 // If your view is enabled then embed it in this page by returning the embed code
 if (isset($view_code)) {
   $output .= $view_code;
 }
 else {
   // Provide the landing page with links to the menu item created in hook_menu to
   // to enable your view
   $output .= '<p>The My Module module uses primarily views to provide an '
     . 'administrative interface. Currently one or more views needed for this '
     . 'administrative interface are disabled. <strong>Click each of the following links to '
     . 'enable the pertinent views</strong>:</p>';
   $output .= '<ul>';
   // NOTE: <URL-FROM-MENU-TO-ENABLE-VIEW> is
   // admin/tripal/<PATH-TO-YOUR-ADMIN-SECTION>/views/<VIEW-MACHINE-NAME>/enable
   // from above hook_menu().
   $output .= '<li>' . l('Genotype Experiments Admin', '<URL-FROM-MENU-TO-ENABLE-VIEW>') . '</li>';
   $output .= '</ul>';
 }

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
      speed up quering of the natural diversity tables which are quite normalized
      and thus require a great number of joins. <strong>It is important to update these
      materialized views whenever you load in new genotype data.</strong>',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );

  $form['mviews']['mview_machine_names'] = array(
    '#type' => 'checkboxes',
    '#title' => '   ',
    '#options' => array(
      'nd_genotype_experiment' => 'ND Genotype Experiment: the foundation for all the functionality
      provided by this module.',
      'nd_genotype_germplasm' => 'ND Genotype Germplasm: listing of germplasm genotyped.',
    )
  );

  $form['mviews']['populate_mviews'] = array(
    '#type' => 'submit',
    '#name' => 'sync_mviews',
    '#value' => 'Sync'
  );

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

  return $form;
}

/**
 * Update ND Genotype data (ie: Materialized Views)
 */
function nd_genotypes_admin_sync_mviews_submit($form, $form_state) {
  global $user;

  switch( $form_state['triggering_element']['#name'] ) {
    case 'sync_mviews':

      foreach ($form_state['values']['mview_machine_names'] as $machine_name => $selected) {
        if ($selected) {
          $mview_id = tripal_get_mview_id($machine_name);
          tripal_refresh_mview($mview_id);

          // If the current view is nd_genotype_experiment then we also need to
          // update the polymorphic information seperatly.
          if ($machine_name == 'nd_genotype_experiment') {
            tripal_add_job(
              "Update polymorphism information for ND Genotypes.",
              'nd_genotypes',
              'nd_genotypes_update_polymorphism_info',
              array($mview_id),
              $user->uid
            );
          }
        }
      }
      break;
    case 'update_template':
      nd_genotypes_add_generic_experiment_bulk_loader(TRUE);
      break;
  }

}

/**
 * Settings for ND Genotypes
 */
function nd_genotypes_admin_settings($form, $form_state) {

  $form['types'] = array(
    '#type' => 'fieldset',
    '#title' => 'Types',
    '#description' => 'Allows you to configure which controlled vocabulary terms
      you would like to fall into each category of features (variants and markers)
      handled by this module.'
  );

  // Get vocab terms for feature types.
  $default_cv = tripal_get_default_cv('feature', 'type_id');
  if (!empty($default_cv)) {
    $feature_types = tripal_get_cvterm_select_options($default_cv->cv_id);
  }
  else {
    tripal_set_message(
      'There is not a default vocabulary set for Feature Types. Please set one
      using the <a href="@url">vocabulary defaults configuration page</a>.',
      TRIPAL_WARNING,
      array('@url' => url('admin/tripal/chado/tripal_cv/defaults'))
    );
    $feature_types = array();
  }

  $form['types']['descrip'] = array(
    '#type' => 'item',
    '#markup' => 'This module assumes the following set-up of data: Sequence
    Backbone > Variants > Markers where variants are the location/range shown
    to vary betweeen germplasm and markers are the actual assays detecting that
    variation. Note that genotypes are only attached to markers (ensures researches
    know the method used) and that both variants & markers have feature locations on
    your sequence backbone (ie: contigs, chromosomes; features with sequence
    information).',
  );

  // Set variant types.
  $form['types']['variants'] = array(
    '#type' => 'select',
    '#title' => 'Variants',
    '#description' => 'Variants are the location/range shown to vary betweeen
      germplasm. They should not have genotypes attached directly to them but
      instead act as a way to see all the markers assaying a particular position.',
    '#multiple' => TRUE,
    '#options' => $feature_types,
    '#default_value' => unserialize(variable_get('nd_genotypes_variant_types', 'a:0:{}'))
  );

  // Set marker types.
  $form['types']['markers'] = array(
    '#type' => 'select',
    '#title' => 'Markers',
    '#description' => 'Markers are the actual assays detecting variation and
      the genotypes detected by each assay would be attached to it.',
    '#multiple' => TRUE,
    '#options' => $feature_types,
    '#default_value' => unserialize(variable_get('nd_genotypes_marker_types', 'a:0:{}'))
  );

  $form['types']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Set Types',
    '#validate' => array('nd_genotypes_admin_settings_types_validate'),
    '#submit' => array('nd_genotypes_admin_settings_types_submit'),
  );

  return $form;
}

/**
 * Config -Types: Validation
 */
function nd_genotypes_admin_settings_types_validate($form, $form_state) {}

/**
 * Config -Types: Submit
 */
function nd_genotypes_admin_settings_types_submit($form, $form_state) {

  variable_set('nd_genotypes_variant_types', serialize($form_state['values']['variants']));
  variable_set('nd_genotypes_marker_types', serialize($form_state['values']['markers']));
}