<?php

/**
 * @file
 * Contains theme functions such as preprocessors
 */

/**
 * Preprocessor: nd_genotypes_feature_genotypes
 */
function nd_genotypes_preprocess_nd_genotypes_feature_genotypes(&$variables) {

  // Defaults.
  $variables['feature'] = $variables['node']->feature;
  $variables['num_alleles'][$variables['feature']->feature_id] = 0;
  $variables['markers'] = array();
  $variables['is_marker'] = FALSE;
  $variables['is_variant'] = FALSE;

  $marker_types = unserialize(variable_get('nd_genotypes_marker_types', 'a:0:{}'));
  $variant_types = unserialize(variable_get('nd_genotypes_variant_types', 'a:0:{}'));

  // If we have a Marker //////////////
  if (in_array($variables['feature']->type_id->cvterm_id, $marker_types)) {

    $variables['is_marker'] = TRUE;
    $variables['markers'][$variables['feature']->feature_id] = $variables['feature'];

    // Get the unique set of alleles for the given marker.
    $variables['alleles'][$variables['feature']->feature_id] = nd_genotypes_get_alleles_for_marker($variables['feature']->feature_id);
    $variables['num_alleles'][$variables['feature']->feature_id] = count($variables['alleles'][$variables['feature']->feature_id]);

  }
  // If we have a Variant /////////////
  elseif (in_array($variables['feature']->type_id->cvterm_id, $variant_types)) {

    $variables['is_variant'] = TRUE;
    $variables['variant'] = $variables['feature'];


    $variant_charts = array();
    $rel_type_name = variable_get('nd_genotypes_rel_type_name', 'is_marker_of');
    $rel_type_name = str_replace('_',' ', $rel_type_name);
    $rel_position = variable_get('nd_genotypes_rel_position', 'object');

    $options = array( 'return_array' => 1 );
    $variables['feature'] = tripal_core_expand_chado_vars($variables['feature'], 'table', 'feature_relationship', $options);

    if (isset($variables['feature']->all_relationships[$rel_position][$rel_type_name])) {
      $relationships = $variables['feature']->all_relationships[$rel_position][$rel_type_name];

      foreach ($relationships as $type_name => $markers) {
        foreach ($markers as $marker) {

          // Determine the identity of the marker.
          if ($rel_position == 'object') {
            $marker_feature_id = $marker->record->subject_id->feature_id;
            $variables['markers'][$marker_feature_id] =  $marker->record->subject_id;
          }
          else {
            $marker_feature_id = $marker->record->object_id->feature_id;
            $variables['markers'][$marker_feature_id] = $marker->record->object_id;
          }

          // Get the genotypes.
          $variables['alleles'][$marker_feature_id] = nd_genotypes_get_alleles_for_marker($marker_feature_id);
          $variables['num_alleles'][$marker_feature_id] = count($variables['alleles'][$marker_feature_id]);

        }
      }
    }
  }
}

/**
 * Preprocessor: nd_genotypes_feature_sequence
 */
function nd_genotypes_preprocess_nd_genotypes_feature_sequence(&$variables) {

  // The maximum length fo sequence to show.
  $max_sequence_length = 500;

  // We don't want to get the sequence for traditionally large types. They are
  // too big,  bog down the web browser, take longer to load and it's not
  // reasonable to print them on a page.
  $residues ='';
  if(strcmp($variables['node']->feature->type_id->name,'scaffold') !=0 and
     strcmp($variables['node']->feature->type_id->name,'chromosome') !=0 and
     strcmp($variables['node']->feature->type_id->name,'supercontig') !=0 and
     strcmp($variables['node']->feature->type_id->name,'pseudomolecule') !=0) {
    $variables['node']->feature = chado_expand_var($variables['node']->feature,'field','feature.residues');
  }

  // Defaults.
  $variables['type'] = 'sequence';
  $variables['sequence_feature_name'] = $variables['node']->feature->name;
  $variables['fasta_description'] = 'The following sequence is that of the ' . $variables['node']->feature->type_id->name
      . ' <strong>without any variants taken into account</strong>.';
  $variables['marked_description'] = '';
  $variables['sequence'] = $variables['node']->feature->residues;

  $variant_types = unserialize(variable_get('nd_genotypes_variant_types', 'a:0:{}'));
  $marker_types = unserialize(variable_get('nd_genotypes_marker_types', 'a:0:{}'));

  // Current Node: Marker
  if (isset($marker_types[ $variables['node']->feature->type_id->cvterm_id ])) {
    $variables['type'] = 'marker';
  }
  // Current Node: Variant
  elseif (isset($variant_types[ $variables['node']->feature->type_id->cvterm_id ])) {
    $variables['type'] = 'variant';
  }

  if ($variables['node']->feature->residues) {
    if ($variables['node']->feature->seqlen > $max_sequence_length) {
      $variables['sequence'] = substr($variables['node']->feature->residues, 1, $max_sequence_length);
      $variables['sequence_range'] = array(
        'feature_id' => $variables['node']->feature->feature_id,
        'feature_name' => $variables['node']->feature->name,
        'start' => 1,
        'end' => $max_sequence_length,
        'length' => $max_sequence_length,
        'full_sequence' => FALSE
      );
    }
    else {
      $variables['sequence'] = $variables['node']->feature->residues;
      $variables['sequence_range'] = array(
        'feature_id' => $variables['node']->feature->feature_id,
        'feature_name' => $variables['node']->feature->name,
        'start' => 1,
        'end' => $variables['node']->feature->seqlen,
        'length' => $variables['node']->feature->seqlen,
        'full_sequence' => TRUE
      );
    }
  }
  else {

    if ($variables['type'] == 'variant' OR $variables['type'] == 'marker') {

      $variables['sequence_range'] = nd_genotypes_get_variant_sequence(
        $variables['node']->feature,
        $variables['type'],
        array(
          'length' => $max_sequence_length
        )
      );
      $variables['sequence'] = (isset($variables['sequence_range']['sequence'])) ? $variables['sequence_range']['sequence'] : '';

    }
    elseif ($variables['node']->feature->seqlen > 0) {
      // @TODO: Handle substringing of the sequence at the database level.
      tripal_report_error(
        'nd_genotypes',
        TRIPAL_WARNING,
        'Unable to determine the sequence for the :category :name (:id).',
        array(
          ':category' => $feature_category,
          ':name' => $variables['node']->feature->name,
          ':id' => $variables['node']->feature->feature_id
        )
      );
    }
    else {
      tripal_report_error(
        'nd_genotypes',
        TRIPAL_WARNING,
        'Unable to determine the sequence for the :category :name (:id)',
        array(
          ':category' => $feature_category,
          ':name' => $variables['node']->feature->name,
          ':id' => $variables['node']->feature->feature_id
        )
      );
      return '';
    }
  }

  if ($variables['type'] == 'sequence') {
    $variables['fasta_header'] = '>' . $variables['node']->feature->name . ':' . $variables['sequence_range']['start'] . '-' .$variables['sequence_range']['end'];

    // Add the marked-up sequence
    $variables['sequence_range'] = array_merge($variables['sequence_range'], array('sequence' => $variables['sequence']));
    $variables['sequence_with_variants'] = nd_genotypes_markup_sequence_with_variants(
      $variables['node']->feature,
      $variables['sequence_range']
    );
  }
  elseif ($variables['sequence']) {
    $variables['fasta_header'] = '>' . $variables['sequence_range']['feature_name'] . ':' . $variables['sequence_range']['start'] . '-' . $variables['sequence_range']['end'] . ' (' . $variables['node']->feature->uniquename . ': ' . $variables['node']->feature->type_id->name . ')';

    // Add the marked-up sequence
    $variables['sequence_range'] = array_merge($variables['sequence_range'], array('sequence' => $variables['sequence']));
    $variables['sequence_with_variants'] = nd_genotypes_markup_sequence_with_variants(
      $variables['node']->feature,
      $variables['sequence_range'],
      $variables['type']
    );
  }

}
