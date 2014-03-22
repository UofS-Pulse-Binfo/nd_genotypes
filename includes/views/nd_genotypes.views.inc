<?php

/**
 * @file
 * Contains basic views integration code.
 * @see nd_genotypes.install: nd_genotype_experiment_views_integration()
 */

/**
 * Implements hook_views_handlers()
 *
 * Purpose: Register all custom handlers with views
 *   where a handler describes either "the type of field",
 *   "how a field should be filtered", "how a field should be sorted"
 *
 * @return array
 *   An array of handler definitions
 */
function nd_genotypes_views_handlers() {
  $handlers = array(
    'info' => array(
      'path' => drupal_get_path('module', 'nd_genotypes') . '/includes/views/handlers',
    ),
    'handlers' => array(

      // These two handlers go hand-in-hand
      // The first renders the field
      // based on the germplasm provided by the second
      'views_handler_field_germplasm_genotype' => array(
        'parent' => 'views_handler_field_prerender_list',
      ),
      'views_handler_filter_germplasm_genotype_by_stock' => array(
        'parent' => 'views_handler_filter_string',
      ),

      // This filter allows users to add new germplasm genotype field/filters
      // ie: it exposes a form that looks like a germplasm genotype filter
      // and uses those values to create a new filter/field combo with those as defaults
      /**
      'views_handler_filter_add_germplasm_genotype_by_stock_field_filter' => array(
        'parent' => 'views_handler_filter_germplasm_genotype_by_stock'
      ),
      */

      // Allows users to only display polymorphic markers
      // ie: you select 2 germplasm genotype fields and if they
      // have different genotypes for a marker then it's polymorphic
      /**
      'views_handler_filter_germplasm_genotype_polymorphic' => array(
      	'parent' => 'views_handler_filter_string',
      ),
      */

      // Returns the residues of a marker/locus parent feature with the
      // position of the marker/locus highlighted by square brackets and
      // the alleles associated with the current marker/locus listed
      // ie: AATTGGTGACACCGT[A/T]GTGGCACGTGCA
      /**
      'views_handler_field_markedup_parent_residues' => array(
        'parent' => 'views_handler_field',
      ),
      */
    ),
  );

  // Join Handler (postgresql array <=> single)
  /**
    $handler['views_handler_join_chado_aggregator'] = array(
     'parent' => 'views_join',
    );
    include_once('views/handlers/views_handler_join_chado_aggregator.inc');
  */

  return $handlers;
}

/**
 * Implements hook_views_data(): Describe chado/tripal tables & fields to views
 *
 * @return array
 *   A data array which follows the structure outlined in the
 *   views2 documentation for this hook. Essentially, it's an array of table
 *   definitions keyed by chado/tripal table name. Each table definition
 *   includes basic details about the table, fields in that table and
 *   relationships between that table and others (joins)
 */
function nd_genotypes_views_data()  {
  $data = array();

  return $data;
}