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
function nd_genotypes_register_nd_genotype_experiment_summary_all_projects_mview () {

	$view_name = 'nd_genotype_experiment_summary_all_projects';
	$schema_definition = 'stockA_id integer,
												stockA_name varchar(255),
												stockB_id integer,
												stockB_name varchar(255),
												project_id integer,
												project_name varchar(255),
												num_shared_features integer,
												num_polymorphic_features integer,
												num_monomorphic_features integer,
												nd_genotype_experiment_summary_id serial';
	$sql = "SELECT 
      nd1.stock_id as stockA_id, 
      nd1.stock_name as stockA_name, 
      nd2.stock_id as stockB_id, 
      nd2.stock_name as stockB_name, 
      0 as project_id, 
      'ALL' as project_name, 
      count(distinct(nd1.feature_name)) as num_shared, 
      sum(CASE WHEN ndp.is_polymorphic THEN 1 ELSE 0 END) as num_polymorphic, 
      sum(CASE WHEN ndp.is_polymorphic AND ndp.is_polymorphic IS NOT NULL THEN 0 ELSE 1 END) as num_monomophic 
	  FROM nd_genotype_experiment nd1 
	  LEFT JOIN nd_genotype_experiment nd2 ON nd1.feature_id=nd2.feature_id 
	  LEFT JOIN nd_genotype_polymorphism ndp ON ndp.stockA_id=nd1.stock_id 
	  WHERE 
	    nd1.project_id=nd2.project_id AND 
	    ndp.stockB_id=nd2.stock_id AND 
	    ndp.feature_id=nd1.feature_id 
	  GROUP BY 
	    nd1.stock_name, 
	    nd1.stock_id, 
	    nd2.stock_name, 
	    nd2.stock_id";

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
      'nd_genotype_experiment_summary_id',
      // SQL to populate materialized view
      $sql,
      // special index
      ''
    );
    
    //Populate
    $mview_id = tripal_mviews_get_mview_id($view_name);
    tripal_update_mview($mview_id);
}