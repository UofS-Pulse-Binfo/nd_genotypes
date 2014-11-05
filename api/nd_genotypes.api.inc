<?php

/**
 * @file
 * Programmers interface to ND Genotypes
 */

/**
 * Determine whether a given stock has genotypes in the nd_genotype_experiment table.
 *
 * @param $stock_id
 *   The unique ID of the stock you want to check
 *
 * @return
 *   TRUE if there are genotypes available and FALSE otherwise
 */
function tripal_germplasm_has_nd_genotypes($stock_id) {
  $query = "SELECT count(*) as nd_genotypes FROM chado.nd_genotype_experiment WHERE stock_id=:stock_id";
  $result = chado_query($query, array(':stock_id' => $stock_id))->fetchField();

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
function nd_genotypes_get_marker($variant_id, $id_only) {

  $rel_type = variable_get('nd_genotypes_rel_type_id', 0);
  $variant_position = variable_get('nd_genotypes_rel_position', 'subject_id');

  // Retrieve the id using a simple SQL statement to save overhead.
  if ($variant_position == 'subject') {
    $sql = 'SELECT object_id FROM {feature_relationship} WHERE subject_id=:variant_id AND type_id=:type_id';
  }
  else {
    $sql = 'SELECT subject_id FROM {feature_relationship} WHERE object_id=:variant_id AND type_id=:type_id';
  }
  $marker_id = chado_query($sql, array(':variant_id' => $variant_id, ':type_id' => $rel_type))->fetchField();

  if ($id_only) {
    return $marker_id;
  }
  else {
    $marker = chado_generate_var('feature', array('feature_id' => $marker_id));
    return $marker;
  }

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
function nd_genotypes_get_variant($marker_id, $id_only) {

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
 * Retrieves all the markers for a sequence including genotypes.
 *
 * @param $feature_id
 *   The feature_id of the sequence the markers are located on.
 * @param $start
 *   The start of the range markers should be located within.
 * @param $end
 *   The end of the range markers should be located within.
 * @return
 *   An array of markers where the marker feature_id is the key and each member
 *   of the array describes a marker including the alleles recorded for it.
 */
function nd_genotypes_get_markers_for_sequence($feature_id, $start, $end) {

  // We use a chado SQL query to join the feature location information with the
  // marker/alleles stored in the nd_genotype_experiment table.
  // First we want to restrict this set to only those markers located on the
  // passed in backbone sequence.
  $where_clause = 'fl.srcfeature_id=:sequence_feature_id';
  $args = array(':sequence_feature_id' => $feature_id);
  // Then if provided we want to restrict to a given range.
  if ($start > 0) {
    $where_clause .= ' AND fl.fmin > :start';
    $args[':start'] = $start;
  }
  if ($end > 0) {
    $where_clause .= ' AND fl.fmax < :end';
    $args[':end'] = $end;
  }
  // Finally generate the full SQL query based on the above conditions.
  $sql = "SELECT
      fl.feature_id as marker_id,
      cf.nid as marker_nid,
      nd.feature_name as marker_name,
      fl.fmin as start,
      fl.fmax as end,
      nd.genotype_description as allele,
      nd.genotype_type_name as allele_type,
      count(distinct(nd.stock_id)) as num_germplasm
    FROM {nd_genotype_experiment} nd
    LEFT JOIN {featureloc} fl ON fl.feature_id=nd.feature_id
    LEFT JOIN chado_feature cf ON cf.feature_id=nd.feature_id
    WHERE " . $where_clause . "
    GROUP BY fl.feature_id, nd.feature_name, cf.nid, fl.fmin, fl.fmax, genotype_description, genotype_type_name
    ORDER BY fl.feature_id, genotype_type_name ASC, genotype_description ASC";
  $results = chado_query($sql, $args);

  // Now we want to reformat the results to be grouped into markers.
  $markers = array();
  foreach ($results as $r) {

    // Add in the marker details.
    // Note this will get reset for each allele a marker has due to the flat
    // structure of the SQL results. As such make sure to assign each value
    // so you don't reset the alleles array each time.
    $markers[$r->marker_id]['marker_id'] = $r->marker_id;
    $markers[$r->marker_id]['marker_name'] = $r->marker_name;
    $markers[$r->marker_id]['marker_nid'] = $r->marker_nid;
    $markers[$r->marker_id]['start'] = $r->start;
    $markers[$r->marker_id]['end'] = $r->end;

    // Add in the current allele.
    $markers[$r->marker_id]['alleles'][$r->allele]['allele'] = $r->allele;
    $markers[$r->marker_id]['alleles'][$r->allele]['allele_type'] = $r->allele_type;
    $markers[$r->marker_id]['alleles'][$r->allele]['num_germplasm'] = $r->num_germplasm;
  }

  return $markers;
}

/**
 * Retrieves a list of alleles for the given marker.
 *
 * @param $marker_id
 *   The feature_id of the marker to retrieve the alleles of.
 * @return
 *   An array of allele objects.
 */
function nd_genotypes_get_alleles_for_marker($marker_id) {

    $sql = "SELECT genotype_description as allele, genotype_type_name as allele_type, count(distinct(stock_id)) as num_germplasm
      FROM {nd_genotype_experiment}
      WHERE feature_id=:feature_id
      GROUP BY genotype_description, genotype_type_name
      ORDER BY genotype_type_name ASC, genotype_description ASC";
    return chado_query($sql, array(':feature_id' => $marker_id))->fetchAll();
}

/**
 * Get the parent sequence of a marker/variant.
 *
 * @param $feature
 *   The marker/variant feature object.
 * @param $feature_category
 *   Either 'variant' or 'marker' depending on the type of feature.
 * @param $sequence_details
 *   An array containing details about the parent sequence to be looked up.
 *   Most notably this might include a 'start', 'stop', 'length' if you want to
 *   specify a range of sequence to be extracted.
 *
 * @return
 *   An array of sequence details including the following information:
 *    - feature_id: the feature id of the parent sequence.
 *    - feature_name: the name of the parent sequence.
 *    - sequence: the actual parent sequence (possibly a range).
 *    - start: the start of the sequence extracted.
 *    - end: the end coordinate of the sequence extracted.
 *    - length: the length of sequence extracted.
 *    - full_sequence: a boolean indicated whether the full sequence was extracted or not.
 *    - variant_id: the feature_id of the variant.
 *    - marker_id: the feature_id of the marker.
 */
function nd_genotypes_get_variant_sequence($feature, $feature_category, $sequence_details = array()) {
  $new_sequence_details = $sequence_details;

  $sql = 'SELECT loc.srcfeature_id, loc.fmin, loc.fmax, pf.residues, pf.seqlen, pf.name
            FROM {featureloc} loc
            LEFT JOIN {feature} pf ON loc.srcfeature_id=pf.feature_id
            WHERE loc.feature_id=:feature_id';
  $record = chado_query($sql, array(':feature_id' => $feature->feature_id))->fetchObject();

  // If the length is provided then return a stubstring.
  if (isset($sequence_details['length'])) {

    // If a start is not provided then center the range on the variant/marker.
    if (!isset($sequence_details['start'])) {
      $middle = round($sequence_details['length'] / 2);
      $sequence_details['start'] = $record->fmin - $middle;
      if ($sequence_details['start'] <= 0) { $sequence_details['start'] = 1; }
    }

    $new_sequence_details = array(
      'feature_id' => $record->srcfeature_id,
      'feature_name' => $record->name,
      'sequence' => substr($record->residues, $sequence_details['start'], $sequence_details['length']),
      'start' => $sequence_details['start'],
      'end' => $sequence_details['start'] + $sequence_details['length'],
      'length' => $sequence_details['length'],
      'full_sequence' => TRUE
    );
  }
  else {
    $new_sequence_details = array(
      'feature_id' => $record->srcfeature_id,
      'feature_name' => $record->name,
      'sequence' => $record->residues,
      'start' => 1,
      'end' => $record->seqlen,
      'length' => $record->seqlen,
      'full_sequence' => TRUE
    );
  }

  // We also want to add the marker_id & variant_id into the sequence details
  // but we only have one of those currently so use the feature category to
  // determine which one we have and which we need to look up.
  if ($feature_category == 'variant') {
    $new_sequence_details['variant_id'] = $feature->feature_id;
    $new_sequence_details['marker_id'] = nd_genotypes_get_marker($feature->feature_id, TRUE);
  }
  elseif ($feature_category == 'marker') {
    $new_sequence_details['marker_id'] = $feature->feature_id;
    $new_sequence_details['variant_id'] = nd_genotypes_get_variant($feature->feature_id, TRUE);
  }
  else {
    tripal_report_error(
      'nd_genotypes',
      TRIPAL_WARNING,
      'Unsupported feature category (:category) when retrieving the variant seuqence for :name (:id).',
      array(
        ':category' => $feature_category,
        ':name' => $feature->name,
        ':id' => $feature->feature_id
      )
    );
  }

  return $new_sequence_details;
}

/**
 * Mark-up the current sequence feature with any known variants using IUPAC codes.
 *
 * Known Limitation: Cannot support markers with a length greater than 1.
 *
 * @param $feature
 *   The feature object of the current sequence/variant/marker.
 * @param $sequence_details
 *   Details of the base sequence to be marked-up. See nd_genotypes_get_variant_sequence()
 *   for format information.
 * @param $feature_category
 *   The type of feature ('sequence', 'variant' or 'marker' are supported).
 *   This is not the same as $feature->type_id->name.
 *
 * @return
 *   The actual marked-up sequence in string format. Includes HTML.
 */
function nd_genotypes_markup_sequence_with_variants($feature, &$sequence_details, $feature_category = 'sequence') {


  // First we need to get the sequence to be marked-up.
  // Note that we also set some defaults while we're at it such as the sequence
  // and marker feature_id's.
  $sequence_details['marker_id'] = (isset($sequence_details['marker_id'])) ? $sequence_details['marker_id'] : NULL;
  $sequence_details['feature_id'] = (isset($sequence_details['feature_id'])) ? $sequence_details['feature_id'] : $feature->feature_id;
  if (isset($sequence_details['sequence'])) {
    $sequence = str_split($sequence_details['sequence']);
  }
  elseif(isset($feature->residues)) {
    $sequence = str_split($feature->residues);
  }
  elseif ($feature_category != 'sequence') {
    $retrieved_sequence_details = nd_genotypes_get_variant_sequence($feature, $feature_category);
    if (!empty($retrieved_sequence_details['sequence'])) {
      $sequence_details['sequence'] = $retrieved_sequence_details['sequence'];
      $sequence_details['feature_id'] = $retrieved_sequence_details['feature_id'];
      $sequence_details['marker_id'] = $retrieved_sequence_details['marker_id'];
      $sequence = str_split($retrieved_sequence_details['sequence']);
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

  // Now get all the markers with their associated genotypes for the range
  // specified in sequence details.
  if (isset($sequence_details['start']) AND isset($sequence_details['end'])) {
    $markers = nd_genotypes_get_markers_for_sequence($sequence_details['feature_id'], $sequence_details['start'], $sequence_details['end']);

    // Now we can mark-up the sequence with the alleles for each marker.
    foreach ($markers as $marker) {

      // Currently we only support single nucleotide alleles due to the added
      // difficulties of replacing multi-nucleotide positions as well as dealing
      // with multiple alleles per marker call (ie: AA in Golden Gate assays).
      if (abs($marker['end'] - $marker['start']) == 1) {

        // Determine the 0-indexed location of the marker depending on whether
        // we're dealing with a sequence range that doesn't start at the
        // beginning of the sequence and assuming that marker locations
        // are 0-indexed.
        if ($sequence_details['start'] == 1) {
          $marker_loc = $marker['start'];
        }
        else {
          $marker_loc = $marker['start'] - $sequence_details['start'];
        }

        // Get the current allele represented in the sequence. This may simply
        // be the base represented in the sequence or, if 2+ markers assay the
        // same location, this may be either an IUPAC code or an expanded [a/t]
        // allele notation.
        $current_allele = strip_tags($sequence[ $marker_loc ]);

        // We need to futher process the allele if it's in expanded notation.
        if (preg_match('/\[(.*)\]/', $current_allele, $match)) {
          $current_allele_already_expanded = TRUE;
          $current_alleles = explode('/', $match[1]);
          $current_alleles = array_combine($current_alleles, $current_alleles);
          $current_allele = nd_genotypes_get_IUPAC_code(str_replace('/','',$match[1]));

        }
        else {
          $current_allele_already_expanded = FALSE;
          $current_allele= nd_genotypes_expand_IUPAC_code($current_allele);
          $current_alleles = str_split($current_allele);
          $current_alleles = array_combine($current_alleles, $current_alleles);
        }


        // First determine the IUPAC code for the alleles present.
        // Keep in mind that if the feature is variant or marker, we need to check
        // if $marker is $feature because if so we want to use the expanded
        // [a/t] notation. We can just make the check directly without first
        // checking the feature_category because the sequence $feature will never
        // correspond to a $marker and thus IUPAC codes will always be used.
        if ($marker['marker_id'] == $sequence_details['marker_id'] OR $current_allele_already_expanded) {
          // Remove bi-allelic (ie: AA) and duplicates.
          $str_alleles = implode('', array_keys($marker['alleles']));
          $str_alleles = nd_genotypes_expand_IUPAC_code($str_alleles);
          // Transform the marker alleles into an array(allele => allele).
          $alleles = str_split($str_alleles);
          $alleles = array_combine($alleles, $alleles);
          // Combine marker alleles with the current alleles at a given position.
          $alleles = array_merge($alleles, $current_alleles);
          // Format the code as [allele1/allele2].
          $code = '[' . implode('/', $alleles) . ']';
          $class = 'variant-expanded';
        }
        else {
          // Transform the marker alleles into an array(allele => allele).
          $alleles = array_keys($marker['alleles']);
          $alleles = array_combine($alleles, $alleles);
          // Combine marker alleles with the current alleles at a given position.
          $alleles = array_merge($alleles, $current_alleles);
          // Format the code as the matching IUPAC code.
          $str_alleles = implode('', $alleles);
          $code = nd_genotypes_get_IUPAC_code($str_alleles);
          $class = 'variant-IUPAC';
        }

        if ($code) {

          // Generate a helpful tooltip (ie: Marker Name: [a/t] ) to help users
          // who don't have IUPAC codes memorized.
          if ($class == 'variant-expanded') {
            $tooltip = $marker['marker_name'];
          }
          else {
            $tooltip = $marker['marker_name'] . ': ' . '[' . implode('/', $alleles) . ']';
          }

          // Now, replace the existing base at that position with the IUPAC code
          // and link to the marker if the node nid is available. Note that if the
          // base feature is a variant or marker then we want to mark-up the
          // current variant/marker using expanded, non-IUPAC form.
          if ($marker['marker_nid'] > 0) {
            $sequence[ $marker_loc ] =
              '<span class="variant ' . $class . '" title="' . $tooltip . '">'
                  . l($code, 'node/' . $marker['marker_nid'])
              . '</span>';
          }
          else {
            $sequence[ $marker_loc ] = '<span class="variant variant-IUPAC" title="' . $tooltip . '">' . $code . '</span>';
          }
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
          ':name' => $feature->name,
          ':id' => $feature->feature_id
        )
      );
  }

  return implode('', $sequence);
}

/**
 * Handles autocomplete for genotyped germplasm names.
 *
 * @param $organism_genus
 *    The genus of the preferred organism.
 * @param $all_organisms
 *    Whether or not to display germplasm for all organisms (TRUE) or only the
 *    preferred organism (FALSE).
 * @param $string
 *    The part of the string already typed in the textfield
 *
 * @ingroup tripal_core
 */
function nd_genotypes_germplasm_name_to_id_callback($organism_genus, $all_organisms, $string) {
  $matches = array();

  // Stores the Drupal db_select() queries used to build the results.
  // Only the first 20 results are returned therefore once that number is
  // reached, no subsequent queries are executed.
  $queries = array();

  // $string is part of the name from the preferred organism.
  $queries['name_preferred'] = db_select('chado.nd_genotype_germplasm', 'g')
    ->fields('g', array('name', 'genus'))
    ->condition('g.name', '%' . db_like($string) . '%', 'LIKE')
    ->condition('g.genus', $organism_genus, '=')
    ->orderBy('char_length(g.name)','ASC')
    ->orderBy('g.genus','ASC');

  // $string is part of the name of any other organism.
  if ($all_organisms) {
    $queries['name_preferred'] = db_select('chado.nd_genotype_germplasm', 'g')
      ->fields('g', array('name', 'genus'))
      ->condition('g.name', '%' . db_like($string) . '%', 'LIKE')
      ->condition('g.genus', $organism_genus, '!=')
      ->orderBy('char_length(g.name)','ASC')
      ->orderBy('g.genus','ASC');
  }

  // Now compile results based on the above queries.
  foreach ($queries as $query) {

    $result = $query->execute();
    foreach ($result as $row) {
      $key = $row->name;
      if (!$all_organisms) {
        $matches[$key] = $row->name;
      }
      else {
        $matches[$key] = format_string(
          '@genus: @name',
          array(
            '@genus' => check_plain($row->genus),
            '@name' => check_plain($row->name),
          )
        );
      }

      // If we already have 20 results then break;
      if (count($matches) > 20) { break; }
    }

    // Need to break the outer loop too.
    if (count($matches) > 20) { break; }
  }

  // return for JS
  drupal_json_output($matches);
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
    return FALSE;
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
