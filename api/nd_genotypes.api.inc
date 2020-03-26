<?php
/**
 * @file
 * Programmers interface to ND Genotypes
 */

/**
 * Execute a query referencing one of the ND Genotype MViews.
 * This is required to ensure the correct hard-partition is queried so
 * NEVER QUERY THE MVIEW DIRECTLY.
 *
 * @param $partition
 *   The string representation of the partition to query.
 *   Currently partitioning by genus is the only method supported.
 * @param $query
 *   The query to be executed. This should contain :placeholders for user submitted data
 *   and have {curly braces} around chado table names.
 * @param $variables
 *   An array of user submitted variables to substitute in to the :placeholders in the query.
 *   The keys should be the :placeholder.
 */
function ndg_mview_query($partition, $query, $variables = array(), $options = array()) {

  $partition = strtolower($partition);
  $options['return_query'] = (isset($options['return_query'])) ? $options['return_query'] : FALSE;

  // Replace the mview table names with the qualified by genus actual table names.
  $query = str_replace('mview_ndg_calls', 'mview_ndg_'.$partition.'_calls', $query);
  $query = str_replace('mview_ndg_variants', 'mview_ndg_'.$partition.'_variants', $query);

  // @todo: handle mview_ndg_germplasm_genotyped.
  if (!chado_table_exists('mview_ndg_'.$partition.'_calls')) {
    return FALSE;
  }
  if (!chado_table_exists('mview_ndg_'.$partition.'_variants')) {
    return FALSE;
  }

  if ($options['return_query']) {
    return $query;
  }
  else {
    return chado_query($query, $variables);
  }
}

/**
 *
 */
function ndg_mview_tables_exist($partition) {
  $exists = TRUE;

  $partition = strtolower($partition);

  $names = array(
    'mview_ndg_'.$partition.'_calls',
    'mview_ndg_'.$partition.'_variants',
    'mview_ndg_germplasm_genotyped',
  );
  foreach ($names as $curr) {
    $curr_exists = chado_table_exists($curr);
    if (!$curr_exists) return FALSE;
  }
  return TRUE;
}

/**
 * Uses drush to safely copy data quickly. ONLY USE IN TRIPAL JOBS!!
 *
 * PostgreSQL COPY is extremely effective at copying/inserting large amount of
 * data. This modules uses it to create materialized views to cache queries
 * for faster data access. However, using chado_query('COPY...') poses many
 * security risks in a web setting. We use a combination of drush sql-cli,
 * which opens a psql session, and psql's \copy command to take advantage of
 * the fact that Tripal Jobs are run on the command-line.
 *
 * @param $copy_command
 *   The full PostgreSQL copy command to be executed.
 */
function ndg_remote_chado_copy($copy_command, $args = array()) {

  // Prefix the tables with their correct schema.
  // Chado tables should be enclosed in curly brackets (ie: {feature} )
  // and Drupal tables should be enclosed in square brackets (ie: [tripal_jobs] ).
  $chado_schema_name = tripal_get_schema_name('chado');
  $drupal_schema_name = tripal_get_schema_name('drupal');
  $copy_command = preg_replace('/\{(.*?)\}/', $chado_schema_name.'.$1', $copy_command);
  $copy_command = preg_replace('/\[(\w+)\]/', $drupal_schema_name.'.$1', $copy_command);

  // Substitute any arguements for placeholders.
  foreach ($args as $k => $v) {
    if (is_string($v)) {
      $args[$k] = "'" . $v . "'";
    }
  }
  $copy_command = strtr($copy_command, $args);

  // Allow the same hooks that act on chado_query to act on this query.
  drupal_alter('chado_query', $copy_command, $args);

  // Setup the command within a drush created psql session using \COPY.
  $connection_string = `drush sql-connect`;
  $connection_string = trim($connection_string);
  $command = $connection_string . ' -c "\\' . $copy_command . '"';

  // Execute the command
  return `$command`;
}

/**
 * Update the postgresql version.
 */
function nd_genotypes_recheck_postgresql_version() {

  // Determine the PostgreSQL version.
  $version_string = db_query('SELECT version()')->fetchField();
  $version = NULL;

  // Pull it out via REGEX.
  // @debug drupal_set_message('Your version is reported as "'.$version_string.'".');
  if (preg_match('/PostgreSQL (\d+\.\d+)/', $version_string, $matches)) {
    $version = $matches[1];
  }

  // If it's numeric then change it to a float for better comparison.
  if (is_numeric($version)) {
    $version = floatval($version);
  }

  variable_set('pgsql_version', $version);

}

/**
 * Retrieve the postgresql version.
 */
function nd_genotypes_get_postgresql_version() {
  return variable_get('pgsql_version', nd_genotypes_recheck_postgresql_version());
}

/**
 * Determine the consensus call for a set of alleles.
 * Currently this is just based on the most frequent one.
 * @todo: Include quality information.
 *
 * @param $alleles
 *   An array of alleles.
 * @return
 *   The consensus call.
 */
function nd_genotype_get_consensus_call($alleles) {

  $counts = array_count_values($alleles);
  arsort($counts);
  reset($counts);
  return key($counts);
}

/**
 * Determine whether a given stock/germplasm has genotypes in the genotype materialized view.
 *
 * @param $stock_id
 *   The unique ID of the stock you want to check
 *
 * @return
 *   TRUE if there are genotypes available and FALSE otherwise
 */
function tripal_germplasm_has_nd_genotypes($stock_id, $partition = 'All') {

  if (!ndg_mview_tables_exist($partition)) {
    return FALSE;
  }

  $query = "SELECT count(*) as nd_genotypes FROM {mview_ndg_calls} WHERE stock_id=:stock_id OR germplasm_id=:stock_id";
  $result = ndg_mview_query($partition, $query, array(':stock_id' => $stock_id))->fetchField();
  if ($result > 0) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Determine whether a given stock/germplasm has genotypes in the genotype materialized view.
 *
 * @param $stock_id
 *   The unique ID of the stock you want to check
 *
 * @return
 *   TRUE if there are genotypes available and FALSE otherwise
 */
function nd_genotypes_has_genotypes($feature_id, $partition = 'All') {

  if (!ndg_mview_tables_exist($partition)) {
    return FALSE;
  }

  $query = "SELECT count(*) as nd_genotypes FROM {mview_ndg_calls} WHERE variant_id=:id OR marker_id=:id";
  $result = ndg_mview_query($partition, $query, array(':id' => $feature_id))->fetchField();
  if ($result > 0) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Retrieve the marker given the variant.
 *
 * @param $variant_id
 *   The feature_id of the variant.
 * @param $id_only
 *   A boolean to indicate return the marker feature_id only.
 * @return
 *   By default a feature object for the marker but if $id_only=TRUE then
 *   just the feature_id of the marker.
 */
function nd_genotypes_get_marker($variant_id, $id_only = FALSE, $only_one = TRUE) {

  $rel_type = variable_get('nd_genotypes_rel_type_id', 0);
  $variant_position = variable_get('nd_genotypes_rel_position', 'subject');

  // Retrieve the id using a simple SQL statement to save overhead.
  if ($variant_position == 'subject') {
    $sql = 'SELECT object_id FROM {feature_relationship} WHERE subject_id=:variant_id AND type_id=:type_id';
  }
  else {
    $sql = 'SELECT subject_id FROM {feature_relationship} WHERE object_id=:variant_id AND type_id=:type_id';
  }
  $marker_ids = chado_query($sql, array(':variant_id' => $variant_id, ':type_id' => $rel_type))->fetchCol();
  if ($id_only) {
    if ($only_one) {
      return array_pop($marker_ids);
    }
    else {
      return $marker_ids;
    }
  }
  else {
    $markers = array();
    foreach ($marker_ids as $marker_id) {
      if ($only_one) {
        return chado_generate_var('feature', array('feature_id' => $marker_id));
      }
      else {
        $markers[] = chado_generate_var('feature', array('feature_id' => $marker_id));
      }
    }
    return $markers;
  }
}

/**
 * Retrieve the type_id for a given type used by this module. This function
 * provides an easy way to grab the admin defined type_id or the default
 * if the admin has yet to configure the type_id.
 *
 * @param $type
 *   The type to return the type_id for. One of the following:
 *     - Marker Type: the featureprop.type_id indicating the marker type
 *         (e.g. Exome Capture).
 *     - Variant Type: the featureprop.type_id indicating the variant type
 *         (e.g. SNP, MNP).
 *     - Feature Relationship: the feature_relationship.type_id connecting the
 *         marker => variant.
 *     - Stock Relationship: the stock_relationship.type_id connecting the
 *         sample => germplasm.
 *
 * @return
 *   The type_id if $type is Marker/Variant Type
 *   or an array (keys: type_id, position) if type is Feature/Stock Relationship.
 */
function nd_genotypes_get_type_id($type) {

  $retrieve_type_id_sql = '
    SELECT cvterm_id FROM {cvterm}
    WHERE name=:term
      AND cv_id IN (SELECT cv_id FROM {cv} WHERE name=:cv)';

  switch ($type) {
    case 'Marker Type':
      $type_id = variable_get('nd_genotypes_markerprop_type_id', NULL);
      if (!$type_id) {
        $type_id = chado_query($retrieve_type_id_sql,
          array(':term' => 'additionalType', ':cv' => 'schema'))->fetchField();
      }
      return $type_id;
      break;
    case 'Variant Type':
      $type_id = variable_get('nd_genotypes_variantprop_type_id', NULL);
      if (!$type_id) {
        $type_id = chado_query($retrieve_type_id_sql,
          array(':term' => 'additionalType', ':cv' => 'schema'))->fetchField();
      }
      return $type_id;
      break;
    case 'Feature Relationship':
      $default_type_id = variable_get('nd_genotypes_rel_type_id', NULL);
      $default_position = variable_get('nd_genotypes_rel_position', NULL);
      if ($default_type_id === NULL) {
        $default_type_id = chado_query($retrieve_type_id_sql,
          array(':term' => 'is marker for', ':cv' => 'relationship'))->fetchField();
        $default_position = 'object';
      }
      return array(
        'type_id' => $default_type_id,
        'position' => $default_position,
      );
      break;
    case 'Project Relationship':
      $default_type_id = variable_get('nd_genotypes_subproject_rel_type_id', NULL);
      $default_position = variable_get('nd_genotypes_subproject_rel_position', NULL);
      if ($default_type_id === NULL) {
        $default_type_id = NULL;
        $default_position = NULL;
      }
      return array(
        'type_id' => $default_type_id,
        'position' => $default_position,
      );
      break;
    case 'Stock Relationship':
      $default_type_id = variable_get('nd_genotypes_stock_rel_type_id', NULL);
      $default_position = variable_get('nd_genotypes_stock_rel_position', NULL);
      if ($default_type_id === NULL) {
        $default_type_id = chado_query($retrieve_type_id_sql,
          array(':term' => 'extracted', ':cv' => 'MIXSCV'))->fetchField();
        $default_position = 'subject';
      }
      return array(
        'type_id' => $default_type_id,
        'position' => $default_position,
      );
      break;
  }

  return FALSE;
}

/**
 * Retrieve the marker type of the given marker.
 *
 * @param $marker_id
 *   The feature_id of the variant.
 * @return
 *   The type of marker as stored in the featureprop table.
 */
function nd_genotypes_get_marker_type($marker_id) {
  $value = FALSE;

  $type_id = nd_genotypes_get_type_id('Marker Type');
  if ($type_id) {
    $value = chado_query(
      "SELECT value FROM {featureprop}
        WHERE feature_id=:id AND type_id=:type_id",
        array(
          ':id' => $marker_id,
          ':type_id' => $type_id,
        ))->fetchField();
  }

  return $value;
}

/**
 * Retrieve the variant given the marker.
 *
 * @param $marker_id
 *   The feature_id of the marker.
 * @param $id_only
 *   A boolean to indicate return the variant feature_id only.
 * @return
 *   By default a feature object for the variant but if $id_only=TRUE then
 *   just the feature_id of the variant.
 */
function nd_genotypes_get_variant($marker_id, $id_only = FALSE) {

  $rel_type = variable_get('nd_genotypes_rel_type_id', 0);
  $variant_position = variable_get('nd_genotypes_rel_position', 'subject_id');

  // Retrieve the id using a simple SQL statement to save overhead.
  if ($variant_position == 'object') {
    $sql = 'SELECT object_id FROM {feature_relationship} WHERE subject_id=:marker_id AND type_id=:type_id';
  }
  else {
    $sql = 'SELECT subject_id FROM {feature_relationship} WHERE object_id=:marker_id AND type_id=:type_id';
  }
  $variant_id = chado_query($sql, array(':marker_id' => $marker_id, ':type_id' => $rel_type))->fetchField();
  if ($id_only) {
    return $variant_id;
  }
  else {
    $variant = chado_generate_var('feature', array('feature_id' => $variant_id));
    return $variant;
  }
}

/**
 * Retrieves a list of alleles for the given variant.
 *
 * @param $variant_id
 *   The feature_id of the variant to retrieve the alleles of.
 * @return
 *   An array of allele objects.
 */
function nd_genotypes_get_alleles_for_variant($variant_id, $partition = 'All', $options = array()) {
  $variant = array();

  if (!ndg_mview_tables_exist($partition)) {
    return FALSE;
  }

  // If the option 'single marker' is provided then the $variant_id should
  // be the marker_id and we should only retrieve calls associated with that
  // marker.
  if (isset($options['single marker']) AND $options['single marker'] === TRUE) {
    $sql = "SELECT
      call.variant_id,
      v.name as variant_name,
      call.marker_id,
      call.marker_name,
      call.allele_call as allele,
      count(*) as num_germplasm
    FROM {mview_ndg_calls} call
      LEFT JOIN {feature} v ON v.feature_id=call.variant_id
    WHERE call.marker_id=:id
    GROUP BY call.variant_id, v.name, call.marker_id, call.marker_name, call.allele_call";
    $resource = ndg_mview_query($partition, $sql, array(':id' => $variant_id))->fetchAll();
  }
  else {
    $sql = "SELECT
      call.variant_id,
      v.name as variant_name,
      call.marker_id,
      call.marker_name,
      call.allele_call as allele,
      count(*) as num_germplasm
    FROM {mview_ndg_calls} call
      LEFT JOIN {feature} v ON v.feature_id=call.variant_id
    WHERE call.variant_id=:id
    GROUP BY call.variant_id, v.name, call.marker_id, call.marker_name, call.allele_call";
    $resource = ndg_mview_query($partition, $sql, array(':id' => $variant_id))->fetchAll();
  }

  foreach ($resource as $r) {
    $variant['variant_id'] = $r->variant_id;
    $variant['variant_name'] = $r->variant_name;

    // Add in the current allele.
    if (isset($variant['alleles'][$r->allele])) {
      $variant['alleles'][$r->allele]['num_germplasm'] += $r->num_germplasm;
    }
    else {
      $variant['alleles'][$r->allele]['allele'] = $r->allele;
      $variant['alleles'][$r->allele]['num_germplasm'] = $r->num_germplasm;
    }

    // Add in allele by marker type.
    $variant['marker_alleles'][$r->marker_id][$r->allele]['allele'] = $r->allele;
    $variant['marker_alleles'][$r->marker_id][$r->allele]['num_germplasm'] = $r->num_germplasm;

    // Also store the marker name.
    $variant['markers'][$r->marker_id] = $r->marker_name;

    //Add in the single base nucleotides.
    foreach (str_split($r->allele) as $n) {
      $variant['nucleotides'][$n] = $n;
    }
  }

  // Finally sort the marker alleles array by key.
  if (!empty($variant)) {
    ksort($variant['marker_alleles']);
  }

  return $variant;
}

/**
 * Mark-up the current sequence feature with any known variants using IUPAC codes.
 *
 * Known Limitation: Cannot support markers with a length greater than 1.
 *
 * @param $values
 *   An array with the following information:
 *     - srcfeature_id: the feature.feature_id to which the sequence belongs.
 *     - srcfeature_name: the feature.name to which the sequence belongs.
 *     - variant_id: the feature.feature_id of the variant to highlight.
 *     - sequence: the sequence fragment to be marked-up.
 *     - sequence_start: the start coordinates of the provided sequence
 *         fragment on the srcfeature.
 *     - sequence_end: the end coordinates of the provided sequence fragment
 *         on the srcfeature.
 *     - sequence_strand: the strand of the provided sequence fragment.
 * @param $options
 *   An array of options... currently none are supported.
 * @param $partition
 *   The partition this variant is in.
 *
 * @return
 *   The actual marked-up sequence in string format. Includes HTML.
 */
function nd_genotypes_markup_sequence_with_variants(&$values, $options = array(), $partition = 'All') {

  if (is_object($values)) { $values = (array) $values; }

  $sequence = str_split($values['sequence']);
  $focus_variant_loc = NULL;

  // Now get all the markers with their associated genotypes for the range
  // specified in sequence details.
  if (isset($values['sequence_start']) AND isset($values['sequence_end'])) {
    $variants = nd_genotypes_get_variants_for_sequence(
      $values['srcfeature_name'],
      $values['sequence_start'],
      $values['sequence_end'],
      $partition
    );

    // Now we can mark-up the sequence with the alleles for each marker.
    foreach ($variants as $k => $variant) {
      $variant = (array) $variant;
      // Currently we only support single nucleotide alleles due to the added
      // difficulties of replacing multi-nucleotide positions as well as dealing
      // with multiple alleles per variant call (ie: AA in Golden Gate assays).
      if (abs($variant['end'] - $variant['start']) == 1) {
        // Determine the 0-indexed location of the variant depending on whether
        // we're dealing with a sequence range that doesn't start at the
        // beginning of the sequence and assuming that variant locations
        // are 0-indexed.
        if ($values['sequence_start'] == 1) {
          $variant_loc = $variant['start'];
        }
        else {
          $variant_loc = $variant['start'] - $values['sequence_start'];
        }

        // Since we are showing the calls on a sequence, we need to check that
        // the strand that the calls were made on matches the one we're showing
        // them on so we don't get the case where the Ref: G and the SNP is T/C
        // making it look like there are 3 alleles when there isn't. Thus,
        // complement the alleles if the strands don't match.
        if (!empty($variant['meta_data']->strand) AND ($variant['meta_data']->strand != $values['sequence_strand'])) {
          // First complement the nucleotides that will be used in the marked-up sequence.
          $variant['nucleotides'] = nd_genotypes_complement_calls($variant['nucleotides']);
          // And then complement the calls as they will appear in the tooltip.
          foreach ($variant['alleles'] as $nucleotide => $info) {
            $variant['alleles'][$nucleotide]['allele'] = nd_genotypes_complement_calls($variant['alleles'][$nucleotide]['allele']);
          }
        }

        // Add the reference nucleotide to the nucleotides assayed.
        $ref_allele = strip_tags($sequence[ $variant_loc ]);
        $variant['nucleotides'][$ref_allele] = $ref_allele;

        // Keep in mind, we need to check if $variant is the focal variant
        // because if it is we want to use the expanded [a/t] notation.
        if ($variant['variant_id'] == $values['variant_id']) {
          // Format the code as [allele1/allele2].
          $code = '[' . implode('/', $variant['nucleotides']) . ']';
          $class = 'variant-expanded';
          // This keeps the expanded variant from being overwritten
          // if there's another variant in the same location.
          $focus_variant_loc = $variant_loc;
        }
        // Otherwise this is not the main variant and we would like to use
        // the more condensed IUPAC code notation.
        elseif ($focus_variant_loc != $variant_loc) {
          // Format the code as the matching IUPAC code.
          $str_alleles = implode('', $variant['nucleotides']);
          $code = nd_genotypes_get_IUPAC_code($str_alleles);
          $class = 'variant-IUPAC';
        }
        else {
          $code = FALSE;
        }

        if ($code) {
          // Generate a helpful tooltip (ie: Variant Name: [a/t] ) to help users
          // who don't have IUPAC codes memorized.
          $allele_info = array($ref_allele.'(Reference)');
          $variant['reference_allele'] = $ref_allele;
          if (!isset($variant['alleles']) OR !is_array($variant['alleles'])) { $variant['alleles'] = array(); }
          foreach ($variant['alleles'] as $i) {
            $allele_info[] = $i['allele'] . '(' . $i['num_germplasm'] . ')';
          }
          $tooltip = $variant['variant_name'] . ': ' . implode(', ', $allele_info);
          $variant['tooltip'] = $tooltip;

          // Add the code and index to the variant for use by the calling function.
          $variant['code'] = $code;
          $variant['index'] = $variant_loc;
          $variants[$k] = $variant;

          // Now, replace the existing base at that position with the IUPAC code.
          $sequence[ $variant_loc ] = $code;
        }
      }
    }
  }
  else {
      tripal_report_error(
        'nd_genotypes',
        TRIPAL_WARNING,
        'The start and end coordinates must be provided in order to mark-up :name (:feature_id) sequence.',
        array(
          ':name' => $values['srcfeature_name'],
          ':id' => $values['srcfeature_id'],
        )
      );
  }

  // Save the variants for use by the calling function.
  $values['variants'] = $variants;

  return implode('', $sequence);
}

/**
 * Retrieves all the variants for a sequence including genotypes.
 *
 * @param $feature_id
 *   The feature_id of the sequence the variants are located on.
 * @param $start
 *   The start of the range variants should be located within.
 * @param $end
 *   The end of the range variants should be located within.
 * @return
 *   An array of variants where the variant feature_id is the key and each member
 *   of the array describes a variant including the alleles recorded for it.
 */
function nd_genotypes_get_variants_for_sequence($srcfeature_name, $start, $end, $partition = 'All') {
  $variants = array();

  if (!ndg_mview_tables_exist($partition)) {
    return FALSE;
  }

  // First get a list of variants.
  $sql = 'SELECT
            v.variant_id,
            v.variant_name,
            v.fmin as start,
            v.fmax as end,
            v.meta_data
          FROM {mview_ndg_variants} v
          WHERE srcfeature_name=:backbone AND fmin >= :min AND fmax <= :max';
  $variants = ndg_mview_query($partition, $sql,
    array(':backbone' => $srcfeature_name, ':min' => $start, ':max' => $end))->fetchAllAssoc('variant_id');

  // If there are no variants then return and empty array
  // obviously there won't be any calls.
  if (empty($variants)) return $variants;

  // Now use that list fo get the allele distribution.
  // If there is only a single variant then we need to use a different query
  // to ensure that chado_query doesn't expand ARRAY[####] as a drupal table.
  if (sizeof($variants) == 1) {
    $sql = 'SELECT
              variant_id,
              allele_call as allele,
              count(*) as num_germplasm
            FROM {mview_ndg_calls} call
            WHERE variant_id='.key($variants).'
            GROUP BY variant_id, allele_call';
  }
  else {
    $sql = 'SELECT
              variant_id,
              allele_call as allele,
              count(*) as num_germplasm
            FROM unnest(ARRAY[' . implode(',', array_keys($variants)) . ']) variant_id
            JOIN {mview_ndg_calls} call USING(variant_id)
            GROUP BY variant_id, allele_call';
  }

  $resource = ndg_mview_query($partition, $sql);
  foreach ($resource as $r) {

    if (is_object($variants[$r->variant_id])) {
      $variants[$r->variant_id] = (array) $variants[$r->variant_id];
      $variants[$r->variant_id]['meta_data'] = json_decode($variants[$r->variant_id]['meta_data']);
    }

    // Add in the current allele.
    $variants[$r->variant_id]['alleles'][$r->allele]['allele'] = $r->allele;
    $variants[$r->variant_id]['alleles'][$r->allele]['num_germplasm'] = $r->num_germplasm;

    //Add in the single base nucleotides.
    foreach (str_split($r->allele) as $n) {
      $variants[$r->variant_id]['nucleotides'][$n] = $n;
    }

  }

  return $variants;
}

/**
 * Complement nucleotides.
 *
 * @param $calls
 *   Either 1) an array of calls to be complemented or 2) a single call to be complemented.
 * @return
 *   The complemented call or calls.
 */
function nd_genotypes_complement_calls($calls) {

  $complement = array(
    'A' => 'T',
    'T' => 'A',
    'C' => 'G',
    'G' => 'C',
  );

  // If I have an array of calls them complement each one.
  if (is_array($calls)) {
    $comp_calls = array();
    foreach ($calls as $allele) {
      if (isset($complement[$allele])) {
        $comp = $complement[$allele];
        $comp_calls[ $comp ] = $comp;
      }
    }

    return $comp_calls;
  }
  // Otherwise, if I have a single allele, complement it.
  elseif (is_string($calls) AND isset($complement[$calls])) {
    return $complement[$calls];
  }

  return FALSE;
}

/**
 * Determine the IUPAC code for a specified set of SNP alleles.
 *
 * @param $alleles
 *   A string contains all the alleles concatenated together.
 * @return
 *   The IUPAC code for that particular allele combination.
 */
function nd_genotypes_get_IUPAC_code($alleles) {
  $IUPAC = array(
        'A' => 'A',
        'T' => 'T',
        'C' => 'C',
        'G' => 'G',
        'AG' => 'R',
        'GA' => 'R',
        'CT' => 'Y',
        'TC' => 'Y',
        'GC' => 'S',
        'CG' => 'S',
        'AT' => 'W',
        'TA' => 'W',
        'GT' => 'K',
        'TG' => 'K',
        'AC' => 'M',
        'CA' => 'M',
        'CGT' => 'B',
        'CTG' => 'B',
        'GCT' => 'B',
        'GTC' => 'B',
        'TGC' => 'B',
        'TCG' => 'B',
        'AGT' => 'D',
        'ATG' => 'D',
        'GAT' => 'D',
        'GTA' => 'D',
        'TGA' => 'D',
        'TAG' => 'D',
        'ACT' => 'H',
        'ATC' => 'H',
        'CAT' => 'H',
        'CTA' => 'H',
        'TCA' => 'H',
        'TAC' => 'H',
        'ACG' => 'V',
        'AGC' => 'V',
        'CAG' => 'V',
        'CGA' => 'V',
        'GAC' => 'V',
        'GCA' => 'V',
      );
  // First expand any IUPAC codes in the alleles
  $alleles = nd_genotypes_expand_IUPAC_code($alleles);
  if (isset($IUPAC[$alleles])) {
    return $IUPAC[$alleles];
  }
  else {
    return 'N';
  }
}

/**
 * Expands any IUPAC codes into individual bases and removes redundant alleles.
 *
 * Since A, T, G , C are also IUPAC codes, this function can be used simply
 * to remove reduntant alleles even if you don't suspect any mixed base alleles.
 * It can also be used to condense bi-allelic genotypes (ie: AA to A).
 *
 * @param $alleles
 *   A string contains all the alleles concatenated together.
 * @return
 *   The IUPAC code for that particular allele combination.
 */
function nd_genotypes_expand_IUPAC_code($codes) {
  $IUPAC = array(
    'A'	=>	'A',
    'T'	=>	'T',
    'C'	=>	'C',
    'G'	=>	'G',
    'R'	=>	'AG',
    'Y'	=>	'CT',
    'S'	=>	'GC',
    'W'	=>	'AT',
    'K'	=>	'GT',
    'M'	=>	'AC',
    'B'	=>	'CGT',
    'D'	=>	'AGT',
    'H'	=>	'ACT',
    'V'	=>	'ACG',
  );
  // Replace each character in the string with it's IUPAC code.
  foreach (str_split($codes) as $c) {
    if (isset($IUPAC[$c])) {
      $codes = str_replace($c, $IUPAC[$c], $codes);
    }
  }
  // Remove any duplicates.
  // NOTE: when mode=3 for count_chars() then a string containing all unique
  // characters is returned as per the PHP documentation.
  $codes = count_chars($codes, 3);
  return $codes;
}

/**
 * Default colours for charts.
 */
function nd_genotypes_set_default_colours() {

  $SNP_colours = array(
    'AA' => '#58A600',
    'TT' => '#FD0000',
    'CC' => '#008AE0',
    'GG' => '#1C1C1C',
    'AT' => '#FF8000',
    'AG' => '#1D5D02',
    'AC' => '#0E6E6C',
    'TG' => '#800000',
    'TC' => '#800080',
    'GC' => '#000080',
  );
  if (variable_get('nd_genotypes_SNP_colours', NULL) === NULL) {
    variable_set('nd_genotypes_SNP_colours', serialize($SNP_colours));
  }

  $allele_colours = array(
    '#1660A8', '#FF6C00', '#259314', '#CC161A', '#804CB3', '#794439', '#DB5CB8', '#6C6C6C', '#AFB400', '#14B1C6',
    '#9EB9E4', '#FFAF5E', '#88DC71', '#FF8482', '#B89ECD', '#B78A81', '#F4A4C9', '#BBBBBB', '#D3D674', '#8BCEDB',
  );
  if (variable_get('nd_genotypes_allele_colours', NULL) === NULL) {
    variable_set('nd_genotypes_allele_colours', serialize($allele_colours));
  }

}

/**
 * Drops all the indexes on a given table.
 *
 * @param $table_name
 *   The table to drop indexes for.
 */
function nd_genotypes_drop_indexes($table_name) {

  $delete_indexes = <<<'EOT'
    DO
      $$BEGIN
        IF EXISTS (select true from pg_index i where i.indrelid = 'chado.TABLENAME'::regclass and i.indisprimary = false)
        THEN
          EXECUTE (
            SELECT 'DROP INDEX ' || string_agg(indexrelid::regclass::text, ', ')
            FROM pg_index  i
              LEFT JOIN pg_depend d ON d.objid = i.indexrelid AND d.deptype = 'i'
            WHERE i.indrelid = 'chado.TABLENAME'::regclass
              AND d.objid IS NULL
          );
        END IF;
      END$$;
EOT;
  $delete_indexes = str_replace('TABLENAME',$table_name, $delete_indexes);
  return chado_query($delete_indexes);

}
