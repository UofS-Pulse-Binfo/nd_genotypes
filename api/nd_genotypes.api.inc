<?php
/**
 * @file
 * Programmers interface to ND Genotypes
 */

/**
 * Determine whether a given stock/germplasm has genotypes in the genotype materialized view.
 *
 * @param $stock_id
 *   The unique ID of the stock you want to check
 *
 * @return
 *   TRUE if there are genotypes available and FALSE otherwise
 */
function tripal_germplasm_has_nd_genotypes($stock_id) {

  $query = "SELECT count(*) as nd_genotypes FROM {genotype_mview} WHERE stock_id=:stock_id OR germplasm_id=:stock_id";
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
function nd_genotypes_get_marker($variant_id, $id_only = FALSE) {

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
 * Retrieve the marker type of the given marker.
 *
 * @param $variant_id
 *   The feature_id of the variant.
 * @return
 *   The type of marker as stored in the featureprop table.
 */
function nd_genotypes_get_marker_type($marker_id) {
  return chado_query("SELECT value FROM {featureprop} WHERE feature_id=:id AND type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='marker_type')",
    array(':id' => $marker_id))->fetchField();
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
function nd_genotypes_get_alleles_for_variant($variant_id) {
  $variant = array();

  $sql = "SELECT
    variant_id,
    variant_name,
    fmin as start,
    fmax as end,
    marker_type,
    allele,
    num_occurances as num_germplasm
  FROM {genotype_counts_mview}
  WHERE variant_id=:id";
  $resource = chado_query($sql, array(':id' => $variant_id))->fetchAll();

  foreach ($resource as $r) {
    $variant['variant_id'] = $r->variant_id;
    $variant['variant_name'] = $r->variant_name;
    $variant['start'] = $r->start;
    $variant['end'] = $r->end;

    // Add in the current allele.
    $variant['alleles'][$r->allele]['allele'] = $r->allele;
    $variant['alleles'][$r->allele]['num_germplasm'] = $r->num_germplasm;

    // Add in allele by marker type.
    if ($r->marker_type != 'All') {
      $variant['marker_alleles'][$r->marker_type][$r->allele]['allele'] = $r->allele;
      $variant['marker_alleles'][$r->marker_type][$r->allele]['num_germplasm'] = $r->num_germplasm;
    }

    //Add in the single base nucleotides.
    foreach (str_split($r->allele) as $n) {
      $variant['nucleotides'][$n] = $n;
    }
  }

  return $variant;
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
    $variants = nd_genotypes_get_variants_for_sequence($sequence_details['feature_name'], $sequence_details['start'], $sequence_details['end']);
    // Now we can mark-up the sequence with the alleles for each marker.
    foreach ($variants as $variant) {
      // Currently we only support single nucleotide alleles due to the added
      // difficulties of replacing multi-nucleotide positions as well as dealing
      // with multiple alleles per variant call (ie: AA in Golden Gate assays).
      if (abs($variant['end'] - $variant['start']) == 1) {
        // Determine the 0-indexed location of the variant depending on whether
        // we're dealing with a sequence range that doesn't start at the
        // beginning of the sequence and assuming that variant locations
        // are 0-indexed.
        if ($sequence_details['start'] == 1) {
          $variant_loc = $variant['start'];
        }
        else {
          $variant_loc = $variant['start'] - $sequence_details['start'];
        }

        // Add the reference nucleotide to the nucleotides assayed.
        $ref_allele = strip_tags($sequence[ $variant_loc ]);
        $variant['nucleotides'][$ref_allele] = $ref_allele;

        // Keep in mind, we need to check if $variant is $feature because if
        // it is we want to use the expanded [a/t] notation.
        if ($variant['variant_id'] == $sequence_details['variant_id']) {
          // Format the code as [allele1/allele2].
          $code = '[' . implode('/', $variant['nucleotides']) . ']';
          $class = 'variant-expanded';
        }
        // Otherwise this is not the main variant and we would like to use
        // the more condensed IUPAC code notation.
        else {
          // Format the code as the matching IUPAC code.
          $str_alleles = implode('', $variant['nucleotides']);
          $code = nd_genotypes_get_IUPAC_code($str_alleles);
          $class = 'variant-IUPAC';
        }

        if ($code) {
          // Generate a helpful tooltip (ie: variant Name: [a/t] ) to help users
          // who don't have IUPAC codes memorized.
          $allele_info = array($ref_allele.'(Reference)');
          foreach ($variant['alleles'] as $i) {
            $allele_info[] = $i['allele'] . '(' . $i['num_germplasm'] . ')';
          }
          $tooltip = $variant['variant_name'] . ': ' . implode(', ', $allele_info);

          // Now, replace the existing base at that position with the IUPAC code
          // and link to the variant if the node nid is available. Note that if the
          // base feature is a variant or variant then we want to mark-up the
          // current variant/variant using expanded, non-IUPAC form.
          if ($variant['variant_nid'] > 0) {
            $sequence[ $variant_loc ] =
              '<span class="variant ' . $class . '" title="' . $tooltip . '">'
                  . l($code, 'node/' . $variant['variant_nid'])
              . '</span>';
          }
          else {
            $sequence[ $variant_loc ] = '<span class="variant variant-IUPAC" title="' . $tooltip . '">' . $code . '</span>';
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

  // Double check that the current variant/marker location is marked-up.
  // Note: We can only do this if we know the position of the current variant/marker ;-).
  if (isset($feature->featureloc->feature_id[0]->fmin)) {
    $variant_loc = $feature->featureloc->feature_id[0]->fmin - $sequence_details['start'];

    if (strpos($sequence[ $variant_loc ], '[') === FALSE) {
      $ref_allele = strip_tags($sequence[ $variant_loc ]);
      $code = '[' . $ref_allele . ']';

      if ($feature->nid > 0) {
        $sequence[ $variant_loc ] =
          '<span class="variant variant-expanded" title="No alleles recorded.">'
              . l($code, 'node/' . $feature->nid)
          . '</span>';
      }
      else {
        $sequence[ $variant_loc ] = '<span class="variant variant-expanded" title="No alleles recorded.">' . $code . '</span>';
      }
      drupal_set_message('We do not have an alternate allele recorded for the current variant/marker.','warning');
    }
  }

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
function nd_genotypes_get_variants_for_sequence($srcfeature_name, $start, $end) {

  $limit = '500';
  $sql = "SELECT
      variant_id,
      variant_name,
      cf.nid as variant_nid,
      fmin as start,
      fmax as end,
      allele,
      num_occurances as num_germplasm
    FROM {genotype_counts_mview}
    LEFT JOIN [chado_feature] cf ON cf.feature_id=variant_id
    WHERE srcfeature_name=:backbone AND fmin >= :min AND marker_type='All'
    ORDER BY fmin ASC
    LIMIT ".$limit;
  $resource = chado_query($sql, array(':backbone' => $srcfeature_name, ':min' => $start));

  $variants = array();
  $r = $resource->fetchObject();
  $offset = 0;
  $times_checked = 1;
  $continue = TRUE;
  do {

    if (!$r) {
      $offset++;
      $tmp_sql  = $sql . ' OFFSET '.($offset * $limit);
      $resource = chado_query($tmp_sql, array(':backbone' => $srcfeature_name, ':min' => $start));
      $times_checked++;
    }
    $r = $resource->fetchObject();

    // Add in the variant details.
    // Note this will get reset for each allele a variant has due to the flat
    // structure of the SQL results. As such make sure to assign each value
    // so you don't reset the alleles array each time.
    if ($r AND $r->end <= $end) {
      $variants[$r->variant_id]['variant_id'] = $r->variant_id;
      $variants[$r->variant_id]['variant_name'] = $r->variant_name;
      $variants[$r->variant_id]['variant_nid'] = $r->variant_nid;
      $variants[$r->variant_id]['start'] = $r->start;
      $variants[$r->variant_id]['end'] = $r->end;
      // Add in the current allele.
      $variants[$r->variant_id]['alleles'][$r->allele]['allele'] = $r->allele;
      $variants[$r->variant_id]['alleles'][$r->allele]['num_germplasm'] = $r->num_germplasm;

      //Add in the single base nucleotides.
      foreach (str_split($r->allele) as $n) {
        $variants[$r->variant_id]['nucleotides'][$n] = $n;
      }
    }

    if (isset($r->end) AND $r->end >= $end) $continue = FALSE;
    if ($times_checked >= 3) $continue = FALSE;

  } while ($continue);

  return $variants;
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
