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

  $form['cvterms'] = array(
    '#type' => 'fieldset',
    '#title' => 'Controlled Vocabulary Terms',
    '#description' => 'Chado uses controlled vocalaries extensively to allow
      for flexible storing of data. As such, this module supports that flexibility
      to ensure that regardless of the types used for your data, this module will
      still be able to navigate the necessary relationships and interpret your types.',
  );

  $form['cvterms']['types'] = array(
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

  $form['cvterms']['types']['descrip'] = array(
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
  $form['cvterms']['types']['variants'] = array(
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
  $form['cvterms']['types']['markers'] = array(
    '#type' => 'select',
    '#title' => 'Markers',
    '#description' => 'Markers are the actual assays detecting variation and
      the genotypes detected by each assay would be attached to it.',
    '#multiple' => TRUE,
    '#options' => $feature_types,
    '#default_value' => unserialize(variable_get('nd_genotypes_marker_types', 'a:0:{}'))
  );

  // Get vocab terms for feature types.
  $default_cv = tripal_get_default_cv('feature_relationship', 'type_id');
  if (!empty($default_cv)) {
    $feature_reltypes = tripal_get_cvterm_select_options($default_cv->cv_id);
  }
  else {
    tripal_set_message(
      'There is not a default vocabulary set for Feature Types. Please set one
      using the <a href="@url">vocabulary defaults configuration page</a>.',
      TRIPAL_WARNING,
      array('@url' => url('admin/tripal/chado/tripal_cv/defaults'))
    );
    $feature_reltypes = array();
  }

  $form['cvterms']['marker_relationship'] = array(
    '#type' => 'fieldset',
    '#title' => 'Variant => Marker Relationship',
    '#description' => 'Since genotypes are only attached to markers, in order to
        display allele calls on your variant pages, this module needs to know
        the relationship connecting your variants to your SNPs.',
  );

  $form['cvterms']['marker_relationship']['reltypes'] = array(
    '#type' => 'hidden',
    '#value' => serialize($feature_reltypes),
  );

  $form['cvterms']['marker_relationship']['marker_reltype'] = array(
    '#type' => 'select',
    '#title' => 'Relationship Type',
    '#description' => 'Indicate the type of relationship connecting your markers
      to the variants they determine.',
    '#options' => $feature_reltypes,
    '#default_value' => variable_get('nd_genotypes_rel_type_id', 0)
  );

  $form['cvterms']['marker_relationship']['marker_relposition'] = array(
    '#type' => 'radios',
    '#title' => 'Variant Position',
    '#description' => 'Since relationships are specified as Subject Type Object
      if you read it like a sentence (ie: SNP54 is_variant_of Markerp25 or
      Markerp25 is_marker_of SNP54), the variant can be either the subject or
      object based on the type you used. As such, we need to know which position
      the variant is in the relationship in order to follow it. Please select
      the position of your variant based on the relationship type provided.',
    '#options' => array(
      'subject' => '<strong>Subject</strong> (Variant is_variant_of Marker)',
      'object' => '<strong>Object</strong> (Marker is_marker_of Variant)'
    ),
    '#default_value' => variable_get('nd_genotypes_rel_position', NULL)
  );

  $form['cvterms']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Set Terms',
    '#validate' => array('nd_genotypes_admin_settings_cvterms_validate'),
    '#submit' => array('nd_genotypes_admin_settings_cvterms_submit'),
  );

  return $form;
}

/**
 * Config -Types: Validation
 */
function nd_genotypes_admin_settings_cvterms_validate($form, $form_state) {}

/**
 * Config -Types: Submit
 */
function nd_genotypes_admin_settings_cvterms_submit($form, $form_state) {

  variable_set('nd_genotypes_variant_types', serialize($form_state['values']['variants']));
  variable_set('nd_genotypes_marker_types', serialize($form_state['values']['markers']));

  $reltypes = unserialize($form_state['values']['reltypes']);
  variable_set('nd_genotypes_rel_type_id', $form_state['values']['marker_reltype']);
  variable_set('nd_genotypes_rel_type_name', $reltypes[ $form_state['values']['marker_reltype'] ]);
  variable_set('nd_genotypes_rel_position', $form_state['values']['marker_relposition']);
}