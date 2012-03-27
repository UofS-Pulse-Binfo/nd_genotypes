<?php

include_once('views/handlers/views_handler_join_chado_aggregator.inc');

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
function kp_genotypes_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'kp_genotypes') . '/views/handlers',
    ),
    'handlers' => array(

      // Join Handler (postgresql array <=> single)
      'views_handler_join_chado_aggregator' => array(
       'parent' => 'views_join',
      ),   
      
    ),
  );
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
  }
  
  return $data;
}

/**
 * Old style Views integration hook for nd_genotype_experiment materialized view
 *
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
    'pre-aggregated' => TRUE,
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
    'pre-aggregated' => TRUE,
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
    'pre-aggregated' => TRUE,
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
    'pre-aggregated' => TRUE,
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
  
  /*.......................................................
   * Beginning of Example Field definitions
   * Remove this section when done

  //Field: plain_text_field (chado datatype)   
  $data['nd_genotype_experiment']['plain_text_field'] = array(
    'title' => t('Human-Readable Name'),
    'help' => t('Description of this field.'),
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

  //Field: numeric_field (chado datatype)   
  $data['nd_genotype_experiment']['numeric_field'] = array(
    'title' => t('Human-Readable Name'),
    'help' => t('Description of this field.'),
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

  //Field: boolean_field (chado datatype)   
  $data['nd_genotype_experiment']['boolean_field'] = array(
    'title' => t('Human-Readable Name'),
    'help' => t('Description of this field.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
    ),
  );

  //Field: unix_timestamp (chado datatype)   
  $data['nd_genotype_experiment']['unix_timestamp'] = array(
    'title' => t('Human-Readable Name'),
    'help' => t('Description of this field.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  //Field: human_readable_date (chado datatype)   
  $data['nd_genotype_experiment']['human_readable_date'] = array(
    'title' => t('Human-Readable Name'),
    'help' => t('Description of this field.'),
    'field' => array(
      'handler' => 'views_handler_field_readble_date',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
  );
   
    * End of Example Field definitions
    */
  
  dpm($data ,'views integration');
  
  return $data;
}