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
function nd_genotypes_register_nd_genotype_polymoprhism_mview () {

	$view_name = 'nd_genotype_polymorphism';
	$schema_definition = 'feature_id integer,
												stockA_id integer,
												stockB_id integer,
												is_polymorphic boolean';
	$sql = "SELECT 
              nd1.feature_id, 
              nd1.stock_id,
              nd2.stock_id,
              (CASE WHEN array_agg(nd1.genotype_description) = array_agg(nd2.genotype_description) THEN FALSE ELSE TRUE END) as is_polymorphic 
          FROM nd_genotype_experiment nd1 
          CROSS JOIN nd_genotype_experiment nd2 
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

		//create
    tripal_add_mview(
      // view name
      $view_name,
      // module creating materialized view
      'nd_genotypes',
      // view table name
      $view_name,
      // schema definition
      $schema_definition,
      // columns to index
      'feature_id, stockA_id, stockB_id',
      // SQL to populate materialized view
      $sql,
      // special index
      ''
    );
    
    //Populate
    $mview_id = tripal_mviews_get_mview_id($view_name);
    tripal_update_mview($mview_id);
}