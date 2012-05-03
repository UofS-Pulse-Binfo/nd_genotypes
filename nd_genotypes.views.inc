<?php

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
      'path' => drupal_get_path('module', 'nd_genotypes') . '/views/handlers',
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
      'views_handler_filter_add_germplasm_genotype_by_stock_field_filter' => array(
        'parent' => 'views_handler_filter_germplasm_genotype_by_stock'
      ),
 
      // Allows users to only display polymorphic markers
      // ie: you select 2 germplasm genotype fields and if they
      // have different genotypes for a marker then it's polymorphic
      'views_handler_filter_germplasm_genotype_polymorphic' => array(
      	'parent' => 'views_handler_filter_string',
      ),
      
      // Returns the residues of a marker/locus parent feature with the
      // position of the marker/locus highlighted by square brackets and
      // the alleles associated with the current marker/locus listed
      // ie: AATTGGTGACACCGT[A/T]GTGGCACGTGCA
      'views_handler_field_markedup_parent_residues' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
  
  if (!module_exists('tripal_views')) {
    // Join Handler (postgresql array <=> single)
    include_once('views/handlers/views_handler_join_chado_aggregator.inc');
    $handlers['handlers']['views_handler_join_chado_aggregator'] = array(
       'parent' => 'views_join',
      );
  }
  
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
  
  // If tripal_views is not enabled/installed but views is
  // then use the old style chado views integration
  if (!module_exists('tripal_views') && module_exists('views')) {
    $data = array_merge($data, nd_genotypes_retrieve_nd_genotype_experiment_views_data());
    $data = array_merge($data, nd_genotypes_retrieve_nd_genotype_experiment_summary_views_data ());
  }

  $data['feature']['markedup_parent_residues'] = array(
    'title' => t('Marked-up Parent Residues'), 
    'help' => t('The sequence the current feature is located on including markup indicating the location of this feature.'), 
    'field' => array(
      'handler' => 'views_handler_field_markedup_parent_residues', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ), 
  );
  
  // REMOVE for Tripal 0.4
  $data['featureloc']['table'] = array(
    'field' => 'featureloc_id', 
    'title' => t('Feature Locations'), 
    'help' => t("Locations of features on each other."), 
    'join' => array(
      'feature' => array(
        'left_field' => 'feature_id',
        'field' => 'feature_id'
      ),
    ),
    'group' => 'Feature Locations',
  );
  
  $data['featureloc']['fmin'] = array(
   'title' => t('Start'), 
    'help' => t('Start'), 
    'field' => array(
      'handler' => 'views_handler_field_numeric', 
      'click sortable' => TRUE,
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['featureloc']['fmax'] = array(
   'title' => t('End'), 
    'help' => t('End'), 
    'field' => array(
      'handler' => 'views_handler_field_numeric', 
      'click sortable' => TRUE,
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
      
  return $data;
}

/**
 * Old style Views integration hook for nd_genotype_experiment materialized view
 */
function nd_genotypes_retrieve_nd_genotype_experiment_views_data () {
  global $db_url;
  $data = array();

  // if the chado database is not local to the drupal database
  // then we need to set the database name.  This should always
  // be 'chado'.
  if(is_array($db_url) and array_key_exists('chado',$db_url)){
    $database = 'chado';
  }
   
  //Basic table definition-----------------------------------
  $data['nd_genotype_experiment']['table']['group'] = t('ND Genotype Experiments');
  
  $data['nd_genotype_experiment']['table']['base'] = array(
    'field' => 'nd_genotype_experiment_id',
    'title' => t('ND Genotype Experiments'),
    'help' => t('An experiment used to determine the allele call for a given locus in a given stock/germplasm.'),
  );
  if($database){
     $data['nd_genotype_experiment']['table']['base']['database'] = $database;
  }

  
  //Relationship Definitions---------------------------------
  //Join: project => nd_genotype_experiment
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['project']['table']['join']['nd_genotype_experiment'] = array(
    'left_field' => 'project_id',
    'field' => 'project_id',
  );  

  //Join: stock => nd_genotype_experiment
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['stock']['table']['join']['nd_genotype_experiment'] = array(
    'left_field' => 'stock_id',
    'field' => 'stock_id',
  ); 

  //Join: nd_genotype_experiment => genotype
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['nd_genotype_experiment']['table']['join']['genotype'] = array(
    'left_field' => 'genotype_id',
    'field' => 'genotype_id',
    'handler' => 'views_handler_join_chado_aggregator',
    'table_aggregated' => 'CURRENT',
  ); 
  
  //Join: genotype => nd_genotype_experiment
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['genotype']['table']['join']['nd_genotype_experiment'] = array(
    'left_field' => 'genotype_id',
    'field' => 'genotype_id',
    'handler' => 'views_handler_join_chado_aggregator',
    'table_aggregated' => 'CURRENT',
  ); 


  //Join: feature => nd_genotype_experiment
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['feature']['table']['join']['nd_genotype_experiment'] = array(
    'left_field' => 'feature_id',
    'field' => 'feature_id',
    'handler' => 'views_handler_join_chado_aggregator',
    'table_aggregated' => 'CURRENT',
  ); 


  //Join: nd_genotype_experiment => feature
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['nd_genotype_experiment']['table']['join']['feature'] = array(
    'left_field' => 'feature_id',
    'field' => 'feature_id',
    'handler' => 'views_handler_join_chado_aggregator',
    'table_aggregated' => 'CURRENT',
  ); 
  
  //Join: nd_experiment => nd_genotype_experiment
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['nd_experiment']['table']['join']['nd_genotype_experiment'] = array(
    'left_field' => 'nd_experiment_id',
    'field' => 'nd_experiment_id',
  ); 

  //Join: nd_geolocation => nd_genotype_experiment
  // Notice that this relationship tells the primary table to show it's fields to the
  // table referencing it by a foreign key and thus the relationship is from
  // primary table to table referenceing it (ie: cvterm => feature)
  $data['nd_geolocation']['table']['join']['nd_genotype_experiment'] = array(
    'left_field' => 'nd_geolocation_id',
    'field' => 'nd_geolocation_id',
  ); 
  
  //Join: nd_genotype_experiment => XY => YYY
  // This relationship should be described in both directions
  // in the appropriate files (ie: for feature => library 
  // describe in both feature.views.inc and library.views.inc)
  /**
  $data['nd_genotype_experiment']['table']['join']['XY'] = array(
    'left_field' => 'matching nd_genotype_experiment key in the XY table',
    'field' => 'nd_genotype_experiment_id',
  );  
  $data['nd_genotype_experiment']['table']['join']['YYY'] = array(
    'left_table' => 'XY',
    'left_field' => 'matching nd_genotype_experiment key in the XY table',
    'field' => 'nd_genotype_experiment_id',
  );
  $data['XY']['table']['join']['YYY'] = array(
    'left_field' => 'primary key in YYY table',
    'field' => 'matching YYY key in the XY table',
  );
  */
   
  //Table Field Definitions----------------------------------
      
  //Field: nd_genotype_experiment_id (primary key)
  $data['nd_genotype_experiment']['nd_genotype_experiment_id'] = array(
    'title' => t('ND Genotype Experiment Primary Key'),
    'help' => t('A unique index for every nd_genotype_experiment.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );  

  //Field: stock_name (chado datatype)   
  $data['nd_genotype_experiment']['stock_name'] = array(
    'title' => t('Stock Name'),
    'help' => t('The human-readable name of the stock this allele was identified in.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  //Field: stock_experiment_relationship_type_name (chado datatype)   
  $data['nd_genotype_experiment']['stock_experiment_relationship_type_name'] = array(
    'title' => t('Stock => Experiment Relationship Type'),
    'help' => t('A term describing the relationship this stock plays in the ND genotype experiment.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  //Field: genotype_description (chado datatype)   
  $data['nd_genotype_experiment']['genotype_description'] = array(
    'title' => t('Allele Call (Genotype)'),
    'help' => t('The allele called at a given locus for a given stock.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  //Field: feature_genotype_relationship_type_name (chado datatype)   
  $data['nd_genotype_experiment']['feature_genotype_relationship_type_name'] = array(
    'title' => t('Feature => Genotype Relationship Type'),
    'help' => t('A term describing the relationship of this genotype to the given feature.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  //Field: feature_name (chado datatype)   
  $data['nd_genotype_experiment']['feature_name'] = array(
    'title' => t('Feature Name'),
    'help' => t('The human-readable name of the locus/marker surveyed during this genotype experiment.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  //Field: nd_geolocation_description (chado datatype)   
  $data['nd_genotype_experiment']['nd_geolocation_description'] = array(
    'title' => t('ND Geolocation Description'),
    'help' => t('A short description of the location this ND genotype experiment was performed at.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  //Field: project_name (chado datatype)   
  $data['nd_genotype_experiment']['project_name'] = array(
    'title' => t('Project Name'),
    'help' => t('The human-readable name of the project including this genotype experiment.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // Calculated Field: genotype.description from nd_genotype_experiment.genotype_description
  // Allows filtering based on germplasm
  $data['nd_genotype_experiment']['germplasm_genotype'] = array(
    'title' => t('Genotypes for Stock'),
    'help' => t('Genotypes of a given stock for each marker.'),
    'field' => array(
      'handler' => 'views_handler_field_germplasm_genotype',
     ),
     'filter' => array(
      'handler' => 'views_handler_filter_germplasm_genotype_by_stock',
     ),
  );
  
  //Expose filter to allow users to add additional germplasm_genotype field/filter combinations
  $data['nd_genotype_experiment']['add_germplasm_genotype'] = array(
    'title' => t('Add Genotypes for Stock'),
    'description' => t('When exposed allows users to dynamically add additional germplasm genotype field/filter combinations.'),
    'filter' => array(
      'handler' => 'views_handler_filter_add_germplasm_genotype_by_stock_field_filter',
    ),
  );

  //Calculated Field: filter to allow pairwise comparison of stock genotypes
  $data['nd_genotype_experiment']['polymorphic'] = array(
    'title' => t('Polymorphic?'),
    'help' => t('Whether a given marker is polymorphic/monomorphic for two stocks.'),
     'filter' => array(
      'handler' => 'views_handler_filter_germplasm_genotype_polymorphic',
     ),
  );
  
  return $data;
}

/**
 * Old style Views integration hook for nd_genotype_experiment_summary materialized view
 */
function nd_genotypes_retrieve_nd_genotype_experiment_summary_views_data () {
  global $db_url;
  $data = array();

  // if the chado database is not local to the drupal database
  // then we need to set the database name.  This should always
  // be 'chado'.
  if(is_array($db_url) and array_key_exists('chado',$db_url)){
    $database = 'chado';
  }
   
  //Basic table definition-----------------------------------
  $data['nd_genotype_experiment_summary']['table']['group'] = t('ND Genotype Experiment Summary');
  
  $data['nd_genotype_experiment_summary']['table']['base'] = array(
    'field' => 'nd_genotype_experiment_summary_id',
    'title' => t('ND Genotype Experiment Summary'),
    'help' => t('Summary statistics regarding natural diversity genotype experiments'),
  );
  if($database){
     $data['nd_genotype_experiment_summary']['table']['base']['database'] = $database;
  }

  $data['nd_genotype_experiment_summary']['stocka_name'] = array(
    'title' => t('StockA Name'),
    'help' => t('The name of the first stock in the comparison.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  $data['nd_genotype_experiment_summary']['stocka_id'] = array(
    'title' => t('StockA ID'),
    'help' => t('The unique identifier of the first stock in the comparison.'),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  
  $data['nd_genotype_experiment_summary']['stockb_name'] = array(
    'title' => t('StockB Name'),
    'help' => t('The name of the second stock in the comparison.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  $data['nd_genotype_experiment_summary']['stockb_id'] = array(
    'title' => t('StockB ID'),
    'help' => t('The unique identifier of the second stock in the comparison.'),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  $data['nd_genotype_experiment_summary']['project_id'] = array(
    'title' => t('Project ID'),
    'help' => t('The unique identifier of the project.'),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
    
  $data['nd_genotype_experiment_summary']['project_name'] = array(
    'title' => t('Project Name'),
    'help' => t('The name of the project genotypes were generated from.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  $data['nd_genotype_experiment_summary']['num_shared_features'] = array(
    'title' => t('Number of Shared Features'),
    'help' => t('The number of unique features with genotypes for both stocks.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['nd_genotype_experiment_summary']['num_polymorphic_features'] = array(
    'title' => t('Number of Polymorphic Features'),
    'help' => t('The number of unique features with different genotypes between stocks.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  ); 
   
  $data['nd_genotype_experiment_summary']['num_monomorphic_features'] = array(
    'title' => t('Number of Monomorphic Features'),
    'help' => t('The number of unique features with the same genotypes between stocks.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  
  return $data;
}