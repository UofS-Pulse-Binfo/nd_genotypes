<?php

/**
 * @file
 * Contains basic views integration code.
 * @see nd_genotypes.install: nd_genotype_experiment_views_integration()
 */

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

  // First check that your integration has not already been added
  // When selecting a priority, do not choose -10 or -9 and make sure it is below 0
  // so that individual sites still have the ability to override it, if needed
  $table_name = 'nd_genotype_experiment';
  $priority = 9;
  if (!tripal_is_table_integrated($table_name, $priority)) {

    // First add our special integration for the nd_genotype_experiment table
    // Although this is integrated by default (priority 10) already, we want to add
    // a more specific integration (priority 9).
    // ------------------------
    tripal_add_views_integration(nd_genotype_experiment_views_integration());

    // We also need to add reciprocal joins to the existing feature, genotype,
    // nd_experiment, nd_geolocation, project and stock integrations so that
    // nd_genotype_experiment fields can be added to views with a base table of
    // these types. These should be relationship only joins.
    // ------------------------
    $tables = array('feature', 'genotype', 'nd_experiment', 'nd_geolocation', 'project', 'stock');
    foreach ($tables as $table) {
      $key = $table . '_id';
      $join = array(
        'base_table' => $table,
        'base_field' => $key,
        'left_table' => 'nd_genotype_experiment',
        'left_field' => $key,
        'relationship_only' => 1,
      );
      tripal_add_join_to_views_integration($table, 7, $join);
    }
  }

  // Since the nd_genotype_germplasm table is a Tripal MView an integration is
  // created for it by default. However, this integration does not expose it
  // as a base table which is how we use it. Thus clone the default integration
  // and set it as a base table.
  $table_name = 'nd_genotype_germplasm';
  $priority = 9;
  if (!tripal_is_table_integrated($table_name, $priority)) {

    $lightest_priority = tripal_get_lightest_views_integration_priority($table_name);
    $setup_id = tripal_clone_views_integration($table_name, $priority, $lightest_priority);

    // And then make a few changes
    // First get the definition array created via the clone above
    $defn_array = tripal_export_views_integration($setup_id);

    // Make this MView a Bast Table so views can be based off of it.
    $defn_array['base_table'] = TRUE;

    // Add joins to the organism and stock table.
    $defn_array['fields']['organism_id']['joins']['organism'] = array(
      'table' => 'organism',
      'field' => 'organism_id',
      'handler' => 'views_join'
    );
    $defn_array['fields']['stock_id']['joins']['stock'] = array(
      'table' => 'stock',
      'field' => 'stock_id',
      'handler' => 'views_join'
    );
    dpm($defn_array, 'germplasm genotyped defn array');

    // And finally save the changes to the integration
    tripal_update_views_integration($setup_id, $defn_array);
  }

  return $data;
}

/**
 * Provide default views integration for the nd_genotype_experiment Materialized View
 */
function nd_genotype_experiment_views_integration () {

 return array(
    'table' => 'nd_genotype_experiment',
    'type' => 'custom',
    'name' => 'Natural Diversity Genotype Experiments',
    'description' => 'List genotype experiments.',
    'priority' => 9,
    'base_table' => TRUE,
    'fields' => array(
      'stock_id' => array(
        'name' => 'stock_id',
        'title' => 'Stock ID',
        'type' => 'integer',
        'description' => 'the unique idenfier for stocks',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(
          'stock' => array(
            'table' => 'stock',
            'field' => 'stock_id',
          ),
        ),
      ),
			'stock_name' => array(
			  'name' => 'stock_name',
			  'title' => 'Stock Name',
			  'type' => 'varchar(255)',
			  'description' => 'Human-readable stock name',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'stock_experiment_relationship_type_id' => array(
			  'name' => 'stock_experiment_relationship_type_id',
			  'title' => 'Stock <=> Experiment Relationship Type ID',
			  'description' => 'The type of relationship joining this stock to this genotype experiment.',
			  'type' => 'integer',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'stock_experiment_relationship_type_name' => array(
			  'name' => 'stock_experiment_relationship_type_name',
			  'title' => 'Stock <=> Experiment Relationship Type',
			  'description' => 'The type of relationship joining this stock to this genotype experiment.',
			  'type' => 'varchar(255)',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'genotype_id' => array(
			  'name' => 'genotype_id',
			  'title' => 'Genotype ID',
			  'description' => 'Unique IDs of the genotypes associated with this experiment',
			  'type' => 'integer[]',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(
          'genotype' => array(
            'table' => 'genotype',
            'field' => 'genotype_id',
          ),
        ),
			),
			'genotype_type_id' => array(
			  'name' => 'genotype_type_id',
			  'title' => 'Genotype Type ID',
			  'description' => 'The ID corresponding to the type of genotype.',
			  'type' => 'integer[]',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'genotype_type_name' => array(
			  'name' => 'genotype_type_name',
			  'title' => 'Genotype Type Name',
			  'description' => 'The human-readable type of genotype.',
			  'type' => 'text[]',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'genotype_uniquename' => array(
			  'name' => 'genotype_uniquename',
			  'title' => 'Genotype Unique Name',
			  'description' => 'Unique name of the genotypes associated with this experiment',
			  'type' => 'text[]',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'genotype_description' => array(
			  'name' => 'genotype_description',
			  'title' => 'Genotype Allele',
			  'description' => 'The genotypes/alleles associated with this experiment',
			  'type' => 'text[]',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'feature_genotype_relationship_type_id' => array(
			  'name' => 'feature_genotype_relationship_type_id',
			  'title' => 'Feature <=> Genotype Relationship Type ID',
			  'description' => 'The type of relationship this genotype has to this feature',
			  'type' => 'integer[]',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'feature_genotype_relationship_type_name' => array(
			  'name' => 'feature_genotype_relationship_type_name',
			  'title' => 'Feature <=> Genotype Relationship Type',
			  'description' => 'The type of relationship this genotype has to this feature',
			  'type' => 'varchar(255)[]',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'feature_id' => array(
			  'name' => 'feature_id',
			  'title' => 'Feature ID',
			  'description' => 'Unique ID for features associated with this genotype experiment.',
			  'type' => 'integer[]',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(
          'feature' => array(
            'table' => 'feature',
            'field' => 'feature_id',
          ),
        ),
			),
			'feature_name' => array(
			  'name' => 'feature_name',
			  'title' => 'Feature Name',
			  'description' => 'Human-readable name of the features associated with this genotype experiment.',
			  'type' => 'varchar(255)[]',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'nd_experiment_id' => array(
			  'name' => 'nd_experiment_id',
			  'title' => 'ND Experiment ID',
			  'description' => 'Unique ID for this genotype experiment.',
			  'type' => 'integer',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(
          'nd_experiment' => array(
            'table' => 'nd_experiment',
            'field' => 'nd_experiment_id'
          ),
        ),
			),
			'nd_geolocation_id' => array(
			  'name' => 'nd_geolocation_id',
			  'title' => 'ND Geolocation ID',
			  'description' => 'Unique ID for the location in which this experiment was done.',
			  'type' => 'integer',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(
          'nd_geolocation' => array(
            'table' => 'nd_geolocation',
            'field' => 'nd_geolocation_id',
          ),
        ),
			),
			'nd_geolocation_description' => array(
			  'name' => 'nd_geolocation_description',
			  'title' => 'ND Geolocation Description',
			  'description' => 'A short description of the location where this experiment was done',
			  'type' => 'varchar(255)',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'project_id' => array(
			  'name' => 'project_id',
			  'title' => 'Project ID',
			  'description' => 'Unique ID of the project this experiment was part of',
			  'type' => 'integer',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(
          'project' => array(
            'table' => 'project',
            'field' => 'project_id',
          ),
        ),
			),
			'project_name' => array(
			  'name' => 'project_name',
			  'title' => 'Project Name',
			  'description' => 'Human-readable name of the project this experiment was part of',
			  'type' => 'varchar(255)',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'polymorphic_with' => array(
			  'name' => 'polymorphic_with',
			  'title' => 'Polymorphic With',
			  'description' => 'The stock_id of stocks with a different allele than the current one.',
			  'type' => 'varchar(255)',
			  'handlers' => array(
          'field' => array('name' => 'views_handler_field'),
          'filter' => array('name' => 'views_handler_filter_string'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),
			'nd_genotype_experiment_id' => array(
			  'name' => 'nd_genotype_experiment_id',
			  'title' => 'ND Genotype Experiment ID',
			  'description' => 'Unique ID of the Genotype Experiment',
			  'type' => 'serial',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_numeric'),
          'filter' => array('name' => 'views_handler_filter_numeric'),
          'sort' => array('name' => 'views_handler_sort'),
        ),
        'joins' => array(),
			),

			// Calculated Field: genotype.description from nd_genotype_experiment.genotype_description
      // Allows filtering based on germplasm
      'germplasm_genotype' => array(
        'name' => 'germplasm_genotype',
        'title' => 'Genotypes for Stock',
        'description' => 'Genotypes of a given stock for each marker.',
        'type' => 'text',
        'handlers' => array(
          'field' => array('name' => 'views_handler_field_germplasm_genotype'),
          'filter' => array('name' => 'views_handler_filter_germplasm_genotype_by_stock'),
        ),
        'joins' => array(),
      ),

      // Calculated Filter: Allows dynamic addition of multiple germplasm_genotype fields
      'add_germplasm_genotype' => array(
        'name' => 'add_germplasm_genotype',
        'title' => 'Genotypes for Multiple Stocks',
        'description' => 'Genotypes of a given stock for each marker; form allows specifying multiple stocks.',
        'type' => 'text',
        'handlers' => array(
          'filter' => array('name' => 'nd_genotypes_handler_filter_genotype_by_stock_multiple'),
        ),
        'joins' => array(),
      ),

      // Calculated Filter: Allows dynamic addition of multiple germplasm_genotype fields
      'polymorphic_germplasm' => array(
        'name' => 'polymorphic_germplasm',
        'title' => 'Polymorphic Markers',
        'description' => 'Only show markers polymorphic between two stocks.',
        'type' => 'text',
        'handlers' => array(
          'filter' => array('name' => 'views_handler_filter_germplasm_genotype_polymorphic'),
        ),
        'joins' => array(),
      ),
    ),
  );
}
