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