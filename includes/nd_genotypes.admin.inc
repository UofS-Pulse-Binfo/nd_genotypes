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

  $output .= "<p>The following table summarizes the data currently sync'd by this module. NOTE: Partitions without any data will not be listed.</p>";

  // Simply display the summary table.
  $table = array('header' => array('stat_name' => ' '), 'rows' => array());
  $stats = chado_query('SELECT * FROM {mview_ndg_summary}');
  foreach ($stats as $stat) {
    $table['header'][ $stat->partition ] = ucwords($stat->partition);

    if (!isset($table['rows'][ $stat->stat_id ])) {
      $table['rows'][ $stat->stat_id ] = array(
        $stat->stat_id => $stat->stat_name
      );
    }

    $table['rows'][ $stat->stat_id ][ $stat->partition ] = number_format($stat->value);
  }

  $output .= theme('table', $table);

  return $output;
}

/**
 *
 */
function nd_genotypes_admin_settings($form, $form_state) {

  $form['storage'] = array(
    '#type' => 'fieldset',
    '#title' => 'Storage Method',
  );

  $form['storage']['note'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="messages warning">This module currently only supports Method #2: Genotype Call Custom table.</div>',
  );

  $form['storage']['storage_method'] = array(
    '#type' => 'select',
    '#title' => 'Storage Method',
    '#description' => 'This is the method you are using to store your genotypic data. You can find more information about the supported storage methods on the ' . l('project wiki.', 'https://github.com/UofS-Pulse-Binfo/nd_genotypes/wiki/How-to-Store-your-Data', array('attributes' => array('target' => '_blank'))),
    '#options' => array(
      'nd_exp' => 'Natural Diversity Experiment',
      'genotype_call' => 'Genotype Call Custom Table',
      'stock_genotype' => 'Genotype Stock table',
      'custom' => 'Custom',
    ),
    '#default_value' => 'genotype_call',
    '#disabled' => TRUE,
  );

  $form['storage']['save'] = array(
    '#type' => 'submit',
    '#name' => 'save',
    '#value' => 'Save Method'
  );

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
    '#default_value' => unserialize(variable_get('nd_genotypes_variant_feature_types', 'a:0:{}'))
  );

  // Set marker types.
  $form['cvterms']['types']['markers'] = array(
    '#type' => 'select',
    '#title' => 'Markers',
    '#description' => 'Markers are the actual assays detecting variation and
      the genotypes detected by each assay would be attached to it.',
    '#multiple' => TRUE,
    '#options' => $feature_types,
    '#default_value' => unserialize(variable_get('nd_genotypes_marker_feature_types', 'a:0:{}'))
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
 *
 */
function nd_genotypes_admin_settings_submit($form, $form_state) {

  // Save the storage method for use later.
  variable_set('nd_genotype_storage_method', $form_state['values']['storage_method']);

}

/**
 * Config -Types: Validation
 */
function nd_genotypes_admin_settings_cvterms_validate($form, $form_state) {}

/**
 * Config -Types: Submit
 */
function nd_genotypes_admin_settings_cvterms_submit($form, $form_state) {

  variable_set('nd_genotypes_variant_feature_types', serialize($form_state['values']['variants']));
  variable_set('nd_genotypes_marker_feature_types', serialize($form_state['values']['markers']));

  $reltypes = unserialize($form_state['values']['reltypes']);
  variable_set('nd_genotypes_rel_type_id', $form_state['values']['marker_reltype']);
  variable_set('nd_genotypes_rel_type_name', $reltypes[ $form_state['values']['marker_reltype'] ]);
  variable_set('nd_genotypes_rel_position', $form_state['values']['marker_relposition']);
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

  $sql = "
    SELECT *
    FROM {organism} O
    ORDER BY O.genus, O.species
  ";
  $orgs = chado_query($sql);
  $partitions = array();
  foreach ($orgs as $organism) {
    // @PARTITION: Setting the partitions that can be sync'd.
    $partitions[$organism->genus] = $organism->genus;
  }
  $partitions['ALL'] = 'ALL';

  $form['mviews']['partition'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Partition(s)',
    '#description' => 'Select which partition(s) you would like to sync data for or select <em>"ALL"</em> to sync all partitions.',
    '#options' => $partitions,
  );

  $form['mviews']['populate_mviews'] = array(
    '#type' => 'submit',
    '#name' => 'sync_mviews',
    '#value' => 'Sync'
  );

  return $form;
}

/**
 * Update ND Genotype data (ie: Materialized Views)
 */
function nd_genotypes_admin_sync_mviews_submit($form, $form_state) {

  if ($form_state['triggering_element']['#name'] == 'sync_mviews') {

    // Determine the partitions to sync.
    if (!empty($form_state['values']['partition']['ALL'])) {
      $keys = array_keys($form_state['values']['partition']);
      $partitions = array_combine($keys, $keys);
      unset($partitions['ALL']);
    }
    else {
      $partitions = array_filter($form_state['values']['partition']);
    }

    // Create a Tripal job for syncing the materialized view.
    if (!empty($partitions)) {
      global $user;
      tripal_add_job(
        'Update ND Genotypes MView for '.implode(', ',$partitions),
        'nd_genotypes',
        'nd_genotypes_update_mview',
        array('partitions' => $partitions),
        $user->uid
      );
    }
    else {
      drupal_set_message("No partitions are being sync'd. Please choose the partitions you would like to sync or select 'All' to sync all partitions.", 'warning');
    }
  }

}
