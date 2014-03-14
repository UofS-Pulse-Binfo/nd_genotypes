<?php

/**
 * Register a Materialized View indicating whether the genotypes for one stock/feature combination
 * are different (polymorphic) compared to another stock/feature combination for a given feature
 * (ie: the feature in both combinations is the same).
 *
 * Biology Definition: A list of all unique stock combinations for a each marker (feature) and
 * whether that marker is polymorphic (genotypes are different) between two stocks
 *
 * Columns:
 *   - feature_id: the feature (marker) being considered
 *   - stockA_id: one of the stocks whose genotypes are being compared; the stock with the
 *       smallest stock_id of the pair
 *   - stockB_id: one of the stocks whose genotypes are being compared; the stock with the
 *       largest stock_id of the pair
 *   - is_polymorphic: a boolean (true/false) indicating that there is at least one genotype call
 *       that is different between the two stocks indicated. Consider, for example, that VarietyA
 *       has the following genotypes for MarkerA: G, G, G, A and VarietyB has the following
 *       genotypes for the same marker: G, G, G (Each call corresponds to a separate genotype
 *       experiment). Since VarietyA has one A called and VarietyB doesn't this marker is
 *       polymorphic between VarietyA and VarietyB.
 */
function nd_genotypes_register_nd_genotype_polymoprhism_mview() {

	$name = 'nd_genotype_polymorphism';
	$comment = 'UPDATE BEFORE nd_genotype_summary. Indicates whether the genotypes for one
	  stock/feature combination are different (polymorphic) compared to another stock/feature
	  combination for a given feature';

	$schema = array(
    'description' => $comment,
    'table' => $name,
    'fields' => array(
  	  'feature_id' => array('type' => 'int'),
	  	'stockA_id' => array('type' => 'int'),
		  'stockB_id' => array('type' => 'int'),
		  'is_polymorphic' => array('type' => 'int') //Boolean
		),
		'indexes' => array(
      'nd_genotype_polymorphism_idx1' => array('feature_id'),
      'nd_genotype_polymorphism_idx2' => array('stockA_id'),
      'nd_genotype_polymorphism_idx3' => array('stockB_id'),
    ),
		'foreign keys' => array(
      'feature' => array(
        'table' => 'feature',
        'columns' => array(
          'feature_id' => 'feature_id',
        ),
      ),
      'stock' => array(
        'table' => 'stock',
        'columns' => array(
          'stockA_id' => 'stock_id',
          'stockB_id' => 'stock_id',
        ),
      ),
    ),
	);

	$query = "SELECT
              nd1.feature_id,
              nd1.stock_id,
              nd2.stock_id,
              (CASE WHEN array_agg(nd1.genotype_description) = array_agg(nd2.genotype_description) THEN 0 ELSE 1 END) as is_polymorphic
          FROM chado.nd_genotype_experiment nd1
          CROSS JOIN chado.nd_genotype_experiment nd2
          WHERE
              nd1.stock_id < nd2.stock_id
              AND nd1.feature_id = nd2.feature_id
              AND nd1.genotype_description IS NOT NULL
              AND nd2.genotype_description IS NOT NULL
          GROUP BY
              nd1.feature_id,
              nd1.stock_id,
              nd2.stock_id
          ORDER BY
              nd1.feature_id,
              is_polymorphic";

  // First check to see if it's already registered/created
  $mview_id = tripal_mviews_get_mview_id($name);
  if (!$mview_id) {

    // If it doesn't already exist then create it
    tripal_add_mview($name, 'nd_genotypes', $schema, $query, $comment);
    $mview_id = tripal_mviews_get_mview_id($name);
  }

  //Populate
  tripal_add_populate_mview($mview_id);
}