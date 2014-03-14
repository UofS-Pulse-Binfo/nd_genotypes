<?php

/**
 * Register a Materialized View for stock=>genotype=>feature data
 *
 * This allows for quicker querying of data by doing the majority of the
 * JOINS ahead of time.
 *
 * Each row in this table corresponds to a genotype observed in a stock.
 * Assumptions:
 *   - 1 nd_experiment_stock per nd_experiment
 *   - 1 nd_experiment_project per nd_experiment
 *   - 1+ nd_experiment_genotype per nd_experiment
 *   - 1 feature per genotype
 *
 * Columns:
 *   - stock_id: unique key of the stock the genotype was sampled from
 *   - stock_name: the human-readable name of the stock the genotype was sampled from
 *   - stock_experiment_relationship_type_id: role stock played in this experiment
 *   - stock_experiment_relationship_type_name: role stock played in this experiment
 *   - genotype_id: unique key of genotype resulting from experiment
 *   - genotype_uniquename: unique name of genotype resulting from experiment
 *   - genotype_description: genotype resulting from experiment
 *   - feature_genotype_relationship_type_id: how genotype is connected to feature
 *   - feature_genotype_relationship_type_name: how genotype is connected to feature
 *   - feature_id: the unique key of the feature the genotype is connected to
 *   - feature_name: the human-readable name of the feature the genotype is connected to
 *   - nd_experiment_id: the genotype experiment unique key
 *   - nd_geolocation_id: unique key of the geolocation where this experiment was performed
 *   - nd_geolocation_description: description of the geolocation where this experiment was performed
 *   - project_id: the unique key of the project this genotype experiment is associated with
 *   - project_name: the name of the project this genotype experiment is associated with
 *   - nd_genotype_experiment_id: unique key for this table
 */
function nd_genotypes_register_nd_genotype_experiment_mview() {

  $name = "nd_genotype_experiment";
  $comment = 'A materialized view consolidating the various components of a natural diversity genotype experiment. All ND Genotype module functionality uses this view. ONLY UPDATE THROUGH '.l('ND Genotypes Administration.', 'admin/tripal/extension/nd_genotypes/mviews');

  $schema = array(
    'description' => $comment,
    'table' => $name,
    'fields' => array(
      'stock_id' => array(
        'type' => 'int'
      ),
      'stock_name' => array(
        'type' => 'varchar',
        'length' => '255'
      ),
      'stock_experiment_relationship_type_id' => array(
        'type' => 'int'
      ),
      'stock_experiment_relationship_type_name' => array(
        'type' => 'varchar',
        'length' => '255'
      ),
      'genotype_id' => array(
        'type' => 'int'
      ),
      'genotype_uniquename' => array(
        'type' => 'text'
      ),
      'genotype_description' => array(
        'type' => 'text'
      ),
      'feature_genotype_relationship_type_id' => array(
        'type' => 'int'
      ),
      'feature_genotype_relationship_type_name' => array(
        'type' => 'varchar',
        'length' => '255'
      ),
      'feature_id' => array(
        'type' => 'int'
      ),
      'feature_name' => array(
        'type' => 'varchar',
        'length' => '255'
      ),
      'nd_experiment_id' => array(
        'type' => 'int'
      ),
      'nd_geolocation_id' => array(
        'type' => 'int'
      ),
      'nd_geolocation_description' => array(
        'type' => 'varchar',
        'length' => '255'
      ),
      'project_id' => array(
        'type' => 'int'
      ),
      'project_name' => array(
        'type' => 'varchar',
        'length' => '255'
      ),
      'polymorphic_with' => array(
        'type' => 'text'
      ),
      'nd_genotype_experiment_id' => array(
        'type' => 'serial'
      ),
    ),
    'indexes' => array(
      'nd_genotype_experiment_idx1' => array('stock_id'),
      'nd_genotype_experiment_idx2' => array('genotype_id'),
      'nd_genotype_experiment_idx3' => array('feature_id'),
      'nd_genotype_experiment_idx4' => array('nd_experiment_id'),
      'nd_genotype_experiment_idx5' => array('project_id'),
    ),
  );

  $query = "SELECT
						s.stock_id,
						s.name as stock_name,
						ndes.type_id as stock_experiment_relationship_type_id,
						ndes_cvt.name as stock_experiment_relationship_type_name,
						g.genotype_id as genotype_id,
						g.uniquename as genotype_uniquename,
						g.description as genotype_description,
						fg.cvterm_id as feature_genotype_relationship_type_id,
						fg_cvt.name as feature_genotype_relationship_type_name,
						f.feature_id as feature_id,
						f.name as feature_name,
						nde.nd_experiment_id,
						nde.nd_geolocation_id,
						ndg.description as nd_geolocation_description,
						p.project_id,
						p.name as project_name
					FROM chado.nd_experiment nde
					LEFT JOIN nd_experiment_project ndep ON ndep.nd_experiment_id=nde.nd_experiment_id
					LEFT JOIN nd_geolocation ndg ON ndg.nd_geolocation_id=nde.nd_geolocation_id
					LEFT JOIN project p ON p.project_id=ndep.project_id
					LEFT JOIN nd_experiment_stock ndes ON ndes.nd_experiment_id=nde.nd_experiment_id
					LEFT JOIN stock s ON s.stock_id=ndes.stock_id
					LEFT JOIN cvterm ndes_cvt ON ndes_cvt.cvterm_id=ndes.type_id
					LEFT JOIN nd_experiment_genotype ndeg ON ndeg.nd_experiment_id=nde.nd_experiment_id
					LEFT JOIN genotype g ON g.genotype_id=ndeg.genotype_id
					LEFT JOIN feature_genotype fg ON fg.genotype_id=g.genotype_id
					LEFT JOIN cvterm fg_cvt ON fg_cvt.cvterm_id=fg.cvterm_id
					LEFT JOIN feature f ON f.feature_id=fg.feature_id";

  // First check to see if it's already registered/created
  $mview_id = tripal_mviews_get_mview_id($name);
  if (!$mview_id) {

    // If it doesn't already exist then create it
    tripal_add_mview($name, 'nd_genotypes', $schema, $query, $comment);
    $mview_id = tripal_mviews_get_mview_id($name);
  }

  // Then use our custom function to update/populate it
  global $user;
  tripal_add_job(
    "Update nd_genotype_experiment Materialized View",
    'nd_genotypes',
    'nd_genotypes_update_nd_genotype_experiment_mview',
    array($mview_id),
    $user->uid
  );

}

/**
 * Custom function to update/populate the nd_genotype_experiment view
 *
 * This is needed because it is a two-step process. Specifically, the polymorphic_with
 * column needs to be updated separately since it relies on other content in the same
 * view.
 */
function nd_genotypes_update_nd_genotype_experiment_mview($mview_id) {

    //Populate
    tripal_populate_mview($mview_id);

    // Update polymorphic_with column
    $update_sql = "UPDATE chado.nd_genotype_experiment SET polymorphic_with=subq.polymorphic_with
      FROM (SELECT poly.feature_id, poly.stockA_id as stock_id, array_agg(poly.stockB_id) as polymorphic_with
       FROM (
         SELECT
             nd1.feature_id,
             nd1.stock_id as stockA_id,
             nd2.stock_id as stockB_id,
             (CASE WHEN array_agg(nd1.genotype_description) = array_agg(nd2.genotype_description) THEN FALSE ELSE TRUE END) as is_polymorphic
         FROM chado.nd_genotype_experiment nd1
         CROSS JOIN chado.nd_genotype_experiment nd2
         WHERE
             nd1.feature_id = nd2.feature_id
             AND nd1.genotype_description IS NOT NULL
             AND nd2.genotype_description IS NOT NULL
         GROUP BY
             nd1.feature_id,
             nd1.stock_id,
             nd2.stock_id
         ORDER BY
             nd1.feature_id,
             nd1.stock_id,
             is_polymorphic
         ) poly
       WHERE
         poly.is_polymorphic
       GROUP BY
         poly.feature_id,
         poly.stockA_id) as subq
      WHERE subq.feature_id=nd_genotype_experiment.feature_id AND subq.stock_id=nd_genotype_experiment.stock_id";
    db_query($update_sql);

}