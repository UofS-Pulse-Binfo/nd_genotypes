<?php

/**
 * Register a Materialized View for counting the number of polymorphisms between stocks
 *
 * This allows for quicker querying of data by doing the majority of the
 * JOINS ahead of time.
 *
 * Each row in this table corresponds to distinct genotype projects involving
 * a given stock pair
 *
 * Columns:
 *   - stockA_id: unique key of one stock of the pair
 *   - stockA_name: the human-readable name of one stock of the pair
 *   - stockA_num_features: the total number of unique features in nd_genotype_experiment
 *   - stockB_id: unique key of one stock of the pair
 *   - stockB_name: the human-readable name of one stock of the pair
 *   - stockB_num_features: the total number of unique features in nd_genotype_experiment
 *   - project_id: the unique key of the project this genotype experiment is associated with
 *   - project_name: the name of the project this genotype experiment is associated with
 *   - num_shared_features: the number of unique features that both stocks have genotypes for
 *   - num_polymorphic_features: the number of unique features that are polymorphic
 *       (different genotypes) between the two stocks
 *   - num_monomorphic_features: the number of unique features that are monomorphic
 *       (same genotypes) between the two stocks
 */
function nd_genotypes_register_nd_genotype_experiment_summary_mview() {

	$name = 'nd_genotype_experiment_summary';
	$comment = 'UPDATE AFTER nd_genotype_polymorphism. Counts the number of polymorphisms between stocks.';

	$schema = array(
    'description' => $comment,
    'table' => $name,
    'fields' => array(
      'stockA_id' => array('type' => 'int'),
      'stockA_name' => array(
        'type' => 'varchar',
        'length' => '255',
      ),
      'stockB_id' => array('type' => 'int'),
      'stockB_name' => array(
        'type' => 'varchar',
        'length' => '255',
      ),
      'project_id' => array('type' => 'int'),
      'project_name' => array(
        'type' => 'varchar',
        'length' => '255',
      ),
      'num_shared_features' => array('type' => 'int'),
      'num_polymorphic_features' => array('type' => 'int'),
      'num_monomorphic_features' => array('type' => 'int'),
      'nd_genotype_experiment_summary_id' => array('type' => 'serial'),
    ),
		'indexes' => array(
      'nd_genotype_experiment_summary_idx1' => array('project_id'),
      'nd_genotype_experiment_summary_idx2' => array('stockA_id'),
      'nd_genotype_experiment_summary_idx3' => array('stockB_id'),
    ),
		'foreign keys' => array(
      'feature' => array(
        'table' => 'project',
        'columns' => array(
          'feature_id' => 'project_id',
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

	$sql_brokedown = "SELECT
      nd1.stock_id as stockA_id,
      nd1.stock_name as stockA_name,
      nd2.stock_id as stockB_id,
      nd2.stock_name as stockB_name,
      nd1.project_id,
      nd1.project_name,
      count(distinct(nd1.feature_name)) as num_shared,
      sum(CASE WHEN ndp.is_polymorphic=1 THEN 1 ELSE 0 END) as num_polymorphic,
      sum(CASE WHEN ndp.is_polymorphic=1 AND ndp.is_polymorphic IS NOT NULL THEN 0 ELSE 1 END) as num_monomophic
	  FROM chado.nd_genotype_experiment nd1
	  LEFT JOIN chado.nd_genotype_experiment nd2 ON nd1.feature_id=nd2.feature_id
	  LEFT JOIN chado.nd_genotype_polymorphism ndp ON ndp.stockA_id=nd1.stock_id
	  WHERE
	    nd1.project_id=nd2.project_id AND
	    ndp.stockB_id=nd2.stock_id AND
	    ndp.feature_id=nd1.feature_id
	  GROUP BY
	    nd1.stock_name,
	    nd1.stock_id,
	    nd2.stock_name,
	    nd2.stock_id,
	    nd1.project_name,
	    nd1.project_id";
  $sql_all_projects = "SELECT
      nd1.stock_id as stockA_id,
      nd1.stock_name as stockA_name,
      nd2.stock_id as stockB_id,
      nd2.stock_name as stockB_name,
      0 as project_id,
      'ALL' as project_name,
      count(distinct(nd1.feature_name)) as num_shared,
      sum(CASE WHEN ndp.is_polymorphic=1 THEN 1 ELSE 0 END) as num_polymorphic,
      sum(CASE WHEN ndp.is_polymorphic=1 AND ndp.is_polymorphic IS NOT NULL THEN 0 ELSE 1 END) as num_monomophic
	  FROM chado.nd_genotype_experiment nd1
	  LEFT JOIN chado.nd_genotype_experiment nd2 ON nd1.feature_id=nd2.feature_id
	  LEFT JOIN chado.nd_genotype_polymorphism ndp ON ndp.stockA_id=nd1.stock_id
	  WHERE
	    nd1.project_id=nd2.project_id AND
	    ndp.stockB_id=nd2.stock_id AND
	    ndp.feature_id=nd1.feature_id
	  GROUP BY
	    nd1.stock_name,
	    nd1.stock_id,
	    nd2.stock_name,
	    nd2.stock_id";

	$query = '('.$sql_brokedown.') UNION ('.$sql_all_projects.')';

  // First check to see if it's already registered/created
  $mview_id = tripal_mviews_get_mview_id($name);
  if (!$mview_id) {

    // If it doesn't already exist then create it
    tripal_add_mview($name, 'nd_genotypes', $schema, $query, $comment);
    $mview_id = tripal_mviews_get_mview_id($name);
  }

  // Populate
  tripal_add_populate_mview($mview_id);
}