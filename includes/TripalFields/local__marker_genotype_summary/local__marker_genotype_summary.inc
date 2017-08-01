<?php
/**
 * @class
 * Purpose: Provides a summary of genotypic data for a given marker.
 *
 * Data:
 *   Genotypic data is expected to be stored in the genotype_call table created
 *   by this module. The design of this table enforces you have one feature
 *   describing the position variation occurs at (the variant) and 1+ features
 *   describing the actual marker assay used to determine the genotypes.
 *
 *   Specifically, this field provides a summary of the genotypic calls for
 *   the current marker.
 *
 * Assumptions:
 *   1. This field should only be attached to a Tripal Content Type storing
 *      it's primary data in the chado.feature table.
 *   2. The feature_id of the current marker will be present in the marker_id
 *      column of the genotype_call table if this marker has any genotypes
 *      attached to it.
 *
 * NOTE: This field pulls all data from the materialized views created by this
 *  module. As such, you must make sure to keep them sync'd in the administrative
 *  UI in order to see any changes after data loading.
 */
class local__marker_genotype_summary extends ChadoField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Genotype Summary';

  // The default description for this field.
  public static $default_description = 'A summary of genotypic data associated with the current marker.';

  // The default widget for this field.
  public static $default_widget = 'local__marker_genotype_summary_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__marker_genotype_summary_formatter';

  // The module that manages this field.
  public static $module = 'nd_genotypes';

  // A list of global settings. These can be accessed within the
  // globalSettingsForm.  When the globalSettingsForm is submitted then
  // Drupal will automatically change these settings for all fields.
  // Once instances exist for a field type then these settings cannot be
  // changed.
  public static $default_settings = array(
    'storage' => 'field_chado_storage',
     // It is expected that all fields set a 'value' in the load() function.
     // In many cases, the value may be an associative array of key/value pairs.
     // In order for Tripal to provide context for all data, the keys should
     // be a controlled vocabulary term (e.g. rdfs:type). Keys in the load()
     // function that are supported by the query() function should be
     // listed here.
     'searchable_keys' => array(),
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'local',
    // The name of the term.
    'term_name' => 'marker_genotype_summary',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'marker_genotype_summary',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asyncronously.
    'auto_attach' => FALSE,

    // Notice that we use the feature.feature_id here. This allows us to take
    // advantage of the ChadoField default class which will load the value of
    // this field for us. We can then use that value to query our materialized views.
    // The table in Chado that the instance maps to.
    'chado_table' => 'feature',
    // The column of the table in Chado where the value of the field comes from.
    'chado_column' => 'feature_id',
    // The base table.
    'base_table' => 'feature',
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;

  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web serivces or downloads.  An example
  // could be a quick search field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = FALSE;

 /**
   * Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the enity object to which this field is attached.
   *     - $field_name is the name of this field
   *     - 'und' is the language code (in this case 'und' == undefined)
   *     - 0 is the cardinality.  Increment by 1 when more than one item is
   *       available.
   *     - 'value' is the key indicating the value of this field. It should
   *       always be set.  The value of the 'value' key will be the contents
   *       used for web services and for downloadable content.  The value
   *       should be of the follow format types: 1) A single value (text,
   *       numeric, etc.) 2) An array of key value pair. 3) If multiple entries
   *       then cardinality should incremented and format types 1 and 2 should
   *       be used for each item.
   *   The array may contain as many other keys at the same level as 'value'
   *   but those keys are for internal field use and are not considered the
   *   value of the field.
   */
  public function load($entity) {

    // ChadoFields automatically load the chado column specified in the
    // default settings above. If that is all you need then you don't even
    // need to implement this function. However, if you need to add any
    // additional data to be used in the display, you should add it here.
    parent::load($entity);

    // First, retrieve the chado feature_id/marker_id for the current Tripal Content, as we requested.
    $marker_id = $entity->local__marker_genotype_summary['und'][0]['chado-feature__feature_id'];

    // Retrieve the data using the api.
    $variant = NULL;
    if ($marker_id) {
      $variant = nd_genotypes_get_alleles_for_variant(
        $marker_id,
        'tripalus',
        array('single marker' => TRUE)
      );
    }

    // If there are no genotypes attached to this marker, we would like to
    // indicate that the genotype summary should be empty.
    if (empty($variant)) {
      $entity->local__marker_genotype_summary['und'][0]['value'] = array();
    }
    else {
      $marker_name = $variant['markers'][$marker_id];

      // First store information about the marker so we can lebel the donut/ring.
      $entity->local__marker_genotype_summary['und'][0]['value'] = array();
      $entity->local__marker_genotype_summary['und'][0]['value']['rdfs:label'] = $marker_name;
      $entity->local__marker_genotype_summary['und'][0]['value']['SO:0001645'] = $marker_name;
      $entity->local__marker_genotype_summary['und'][0]['value']['hydra:member'] = array();

      // Then store information about each allele so we can draw/label each
      // part or pie piece of the donut/ring.
      dpm($variant['marker_alleles'][$marker_id], 'members');
      foreach ($variant['marker_alleles'][$marker_id] as $d) {
        $entity->local__marker_genotype_summary['und'][0]['value']['hydra:member'][] = array(
          'SO:0001023' => $d['allele'],
          'rdfs:label' => $d['allele'],
          'local:count' => $d['num_germplasm'],
        );
      }

    }
  }
}
