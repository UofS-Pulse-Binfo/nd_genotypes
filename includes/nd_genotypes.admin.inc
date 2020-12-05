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
  if (chado_table_exists('mview_ndg_summary')) {
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
  }
  else {
    $output .= '<div class="messages warning">No genotypic data has yet been sync\'d. To Sync data, click on the "Sync" tab in the top right corner of this page.</div>';
  }

  return $output;
}

/**
 *
 */
function nd_genotypes_admin_settings($form, $form_state) {

  $form['#attached']['css'][] = array(
    'data' => '
      fieldset.colours .form-type-textfield label {
        display: inline;
        padding-right: 5px;
      }
      fieldset.colours  .swatch {
        float: left;
        margin-top: 10px;
        margin-right: 10px;
        display: block;
        height: 20px;
        width: 20px;
      }
      #edit-swatches {
        height: 60px;
      }
    ',
    'type' => 'inline'
  );

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

  $form['colours'] = array(
    '#type' => 'fieldset',
    '#title' => 'Allele Colours',
    '#description' => 'Allele colours to be used through the fields provided by this module are set here to ensure consistency across fields and provide the best user experience.',
    '#attributes' => array(
      'class' => array('colours'),
    ),
  );

  $form['colours']['SNPs'] = array(
    '#type' => 'fieldset',
    '#title' => 'SNP Alleles',
    '#description' => 'Since SNP alleles are limited to a particular set and SNPs are the preferred variant type, we provide the ability to pick the colour of each allele below. This allows you to ensure that AA is always the same colour when displayed by this module.',
    '#tree' => TRUE,
  );

  nd_genotypes_set_default_colours();
  $default_SNP_colours = unserialize(variable_get('nd_genotypes_SNP_colours'));

  foreach ($default_SNP_colours as $allele => $colour) {
    $form['colours']['SNPs'][$allele] = array(
      '#type' => 'textfield',
      '#title' => $allele,
      '#size' => 16,
      '#default_value' => $colour,
      '#prefix' => '<span class="swatch" style="background-color:'.$colour.'; color:'.$colour.';">'.$allele.'</span>'
    );
  }

  $form['colours']['SNPs']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save SNP Colours',
  );

  $form['colours']['alleles'] = array(
    '#type' => 'fieldset',
    '#title' => 'Generic Alleles',
    '#description' => 'This section allows you to provide a collection of colours to use for alleles that do not fall into the SNP alleles above (e.g. MNPs or indels). When these alleles are detected, each allele will pick one of the following colours in order.',
  );

  $default_allele_colours = unserialize(variable_get('nd_genotypes_allele_colours'));
  $form['colours']['alleles']['allele_colors'] = array(
    '#type' => 'textarea',
    '#title' => 'Catagorical Colour Set',
    '#default_value' => implode('   ', $default_allele_colours),
    '#description' => 'A listing of HEX colour codes seperated by white-space. Colours will be choosen in the order they appear.',
  );

  $form['colours']['alleles']['swatches'] = array(
    '#type' => 'item',
    '#title' => 'Current Colours',
    '#markup' => '',
  );
  foreach ($default_allele_colours as $colour) {
    $form['colours']['alleles']['swatches']['#markup'] .= '<span class="swatch" style="background-color:'.$colour.'; color:'.$colour.';"></span>';
  }

  $form['colours']['alleles']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Generic Allele Colourset',
  );

  $form['cvterms'] = array(
    '#type' => 'fieldset',
    '#title' => 'Controlled Vocabulary Terms',
    '#description' => 'Chado uses controlled vocalaries extensively to allow
      for flexible storing of data. As such, this module supports that flexibility
      to ensure that regardless of the types used for your data, this module will
      still be able to navigate the necessary relationships and interpret your types.',
  );

  $form['cvterms']['featureprop'] = array(
    '#type' => 'fieldset',
    '#title' => 'Feature Properties',
    '#description' => 'The type of variant (e.g. SNP, MNP, etc.) and marker
      (e.g. Exome Capture) is expected to be stored as a feature property of the
      variant and maker respectively. This is where you can indicate the type of
      property you used.',
  );

  $featureprop_types = array();
  $resource = chado_query(
    'SELECT prop.type_id, cvt.name as type_name
     FROM {featureprop} prop
     LEFT JOIN {cvterm} cvt ON cvt.cvterm_id=prop.type_id');
  foreach ($resource as $r) {
    $featureprop_types[ $r->type_id ] = $r->type_name;
  }
  if (empty($featureprop_types)) {
    $msg = 'You need to have specified properties of markers/variants before property types (featureprop.type_id) can be detected.';
    $form['cvterms']['featureprop']['#description'] .= '<div class="messages warning">'.$msg.'</div>';
  }

  $form['cvterms']['featureprop']['markerprop'] = array(
    '#type' => 'select',
    '#title' => 'Marker Type',
    '#description' => 'Indicate the type feature property indicating your marker type (e.g. Exome Capture).',
    '#options' => $featureprop_types,
    '#default_value' => nd_genotypes_get_type_id('Marker Type'),
  );

  $form['cvterms']['featureprop']['variantprop'] = array(
    '#type' => 'select',
    '#title' => 'Variant Type',
    '#description' => 'Indicate the type feature property indicating your variant type (e.g. SNP, MNP, etc.).',
    '#options' => $featureprop_types,
    '#default_value' => nd_genotypes_get_type_id('Variant Type'),
  );

  $form['cvterms']['marker_relationship'] = array(
    '#type' => 'fieldset',
    '#title' => 'Variant => Marker Relationship',
    '#description' => 'Since genotypes are only attached to markers, in order to
        display allele calls on your variant pages, this module needs to know
        the relationship connecting your variants to your SNPs.',
  );

  // Get vocab terms for feature relationship types.
  $feature_reltypes = array();
  $resource = chado_query(
    'SELECT fr.type_id, cvt.name as type_name
     FROM {feature_relationship} fr
     LEFT JOIN {cvterm} cvt ON cvt.cvterm_id=fr.type_id');
  foreach ($resource as $r) {
    $feature_reltypes[ $r->type_id ] = $r->type_name;
  }
  if (empty($feature_reltypes)) {
    $msg = 'You need to have loaded genotypic data before the relationship types can be detected.';
    $form['cvterms']['marker_relationship']['#description'] .= '<div class="messages warning">'.$msg.'</div>';
  }

  $form['cvterms']['marker_relationship']['reltypes'] = array(
    '#type' => 'hidden',
    '#value' => serialize($feature_reltypes),
  );

  // Determine the default.
  $default_feature_rel = nd_genotypes_get_type_id('Feature Relationship');

  $form['cvterms']['marker_relationship']['marker_reltype'] = array(
    '#type' => 'select',
    '#title' => 'Relationship Type',
    '#description' => 'Indicate the type of relationship connecting your markers
      to the variants they determine.',
    '#options' => $feature_reltypes,
    '#default_value' => $default_feature_rel['type_id'],
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
    '#default_value' => $default_feature_rel['position'],
  );

  $form['cvterms']['stock_relationship'] = array(
    '#type' => 'fieldset',
    '#title' => 'Sample => Germplasm Relationship',
    '#description' => 'Since genotypes are only attached to samples, in order to
        display allele calls for a given germplasm, this module needs to know
        the relationship connecting your samples to your germplasm.',
  );

  // Get vocab terms for feature relationship types.
  $stock_reltypes = array();
  $resource = chado_query(
    'SELECT sr.type_id, cvt.name as type_name
     FROM {stock_relationship} sr
     LEFT JOIN {cvterm} cvt ON cvt.cvterm_id=sr.type_id');
  foreach ($resource as $r) {
    $stock_reltypes[ $r->type_id ] = $r->type_name;
  }
  if (empty($stock_reltypes)) {
    $msg = 'You need to have loaded genotypic data before the relationship types can be detected.';
    $form['cvterms']['stock_relationship']['#description'] .= '<div class="messages warning">'.$msg.'</div>';
  }

  $form['cvterms']['stock_relationship']['reltypes'] = array(
    '#type' => 'hidden',
    '#value' => serialize($stock_reltypes),
  );

  // Determine the default.
  $default_stock_rel = nd_genotypes_get_type_id('Stock Relationship');

  $form['cvterms']['stock_relationship']['stock_reltype'] = array(
    '#type' => 'select',
    '#title' => 'Relationship Type',
    '#description' => 'Indicate the type of relationship connecting your samples
      to the germplasm they were extracted from.',
    '#options' => $stock_reltypes,
    '#default_value' => $default_stock_rel['type_id'],
  );

  $form['cvterms']['stock_relationship']['stock_relposition'] = array(
    '#type' => 'radios',
    '#title' => 'Sample Position',
    '#description' => 'Since relationships are specified as Subject Type Object
      if you read it like a sentence (ie: DNA123 is_extracted_from Germplasm5 or
      Germplasm5 is_source_of DNA123), the sample (DNA123 in the examples)
      can be either the subject or object based on the type you used. As such,
      we need to know which position the sample is in the relationship in order
      to follow it. Please select the position of your sample based on the
      relationship type provided.',
    '#options' => array(
      'subject' => '<strong>Subject</strong> (Sample is_extracted_from Germplasm)',
      'object' => '<strong>Object</strong> (Germplasm is_source_of Sample))'
    ),
    '#default_value' => $default_stock_rel['position'],
  );

  $form['cvterms']['project'] = array(
    '#type' => 'fieldset',
    '#title' => 'Projects',
    '#description' => 'Projects are used to group all genotype calls from a single project and/or analysis.<br />',
  );

  $form['cvterms']['project']['follow_project_rels'] = array(
    '#type' => 'checkbox',
    '#title' => 'Follow Project Relationships',
    '#description' => 'If checked, the genotype matrix project filter will expand to sub-projects of the selected project.',
    '#default_value' => variable_get('ndg_follow_project_rels', FALSE),
  );

  // Get vocab terms for project relationship types.
  $project_reltypes = array();
   $resource = chado_query(
    'SELECT pr.type_id, cvt.name as type_name
     FROM {project_relationship} pr
     LEFT JOIN {cvterm} cvt ON cvt.cvterm_id=pr.type_id');
  foreach ($resource as $r) {
    $project_reltypes[ $r->type_id ] = $r->type_name;
  }
  if (empty($project_reltypes)) {
    $msg = 'You need to have created relationships between projects before the relationship types can be detected.';
    $form['cvterms']['project']['#description'] .= '<div class="messages warning">'.$msg.'</div>';
  }

  // Determine the default.
  $default_project_rel = nd_genotypes_get_type_id('Project Relationship');

  $form['cvterms']['project']['subproject_type_id'] = array(
    '#type' => 'select',
    '#title' => 'Sub-Project Relationship Type',
    '#description' => 'Indicate the type of relationship connecting your projects to their sub-projects.',
    '#options' => $project_reltypes,
    '#default_value' => $default_project_rel['type_id'],
  );

  $form['cvterms']['project']['subproject_position'] = array(
    '#type' => 'radios',
    '#title' => 'Parent Project Position',
    '#description' => 'Since relationships are specified as Subject Type Object
      if you read it like a sentence (ie: Master Project is_parent_project_of SubProject or
      SubProject is_subproject_of Master Project), the parent project can be either the subject or
      object based on the type you used. As such, we need to know which position
      the parent project is in the relationship in order to follow it. Please select
      the position of your parent project based on the relationship type provided.',
    '#options' => array(
      'subject' => '<strong>Subject</strong> (Master Project is_parent_project_of SubProject)',
      'object' => '<strong>Object</strong> (SubProject is_subproject_of Master Project)'
    ),
    '#default_value' => $default_project_rel['position'],
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

  // Colours.
  if ($form_state['triggering_element']['#value'] == 'Save SNP Colours') {
    $colours = $form_state['values']['SNPs'];
    unset($colours['submit']);
    variable_set('nd_genotypes_SNP_colours', serialize($colours));
    drupal_set_message('Successfully updated SNP colours.');
  }
  if ($form_state['triggering_element']['#value'] == 'Save Generic Allele Colourset') {
    $colours = preg_split('/\s+/', $form_state['values']['allele_colors']);
    variable_set('nd_genotypes_allele_colours', serialize($colours));
    drupal_set_message('Successfully updated Generic Allele colourset.');
  }

}

/**
 * Config -Types: Validation
 */
function nd_genotypes_admin_settings_cvterms_validate($form, $form_state) {}

/**
 * Config -Types: Submit
 */
function nd_genotypes_admin_settings_cvterms_submit($form, $form_state) {

  // Feature Properties.
  variable_set('nd_genotypes_markerprop_type_id', $form_state['values']['markerprop']);
  variable_set('nd_genotypes_variantprop_type_id', $form_state['values']['variantprop']);

  // Variant => Marker Relationship.
  $reltypes = unserialize($form_state['values']['reltypes']);
  variable_set('nd_genotypes_rel_type_id', $form_state['values']['marker_reltype']);
  variable_set('nd_genotypes_rel_type_name', $reltypes[ $form_state['values']['marker_reltype'] ]);
  variable_set('nd_genotypes_rel_position', $form_state['values']['marker_relposition']);

  // Sample => Germplasm Relationship.
  variable_set('nd_genotypes_stock_rel_type_id', $form_state['values']['stock_reltype']);
  variable_set('nd_genotypes_stock_rel_position', $form_state['values']['stock_relposition']);

  // Projects.
  variable_set('ndg_follow_project_rels', $form_state['values']['follow_project_rels']);
  variable_set('nd_genotypes_subproject_rel_type_id', $form_state['values']['subproject_type_id']);
  variable_set('nd_genotypes_subproject_rel_position', $form_state['values']['subproject_position']);
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
      materialized views whenever you load in new genotype data.</strong>
      <div class="messages notice">It is important the sync Tripal job be run on the command-line through Drush using the command displayed when you submit this form.</div>',
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
