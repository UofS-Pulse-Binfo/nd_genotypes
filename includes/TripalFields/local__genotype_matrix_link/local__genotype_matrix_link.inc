<?php
/**
 * @class
 * Purpose:
 *
 * Data:
 * Assumptions:
 */
class local__genotype_matrix_link extends ChadoField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Genotype Matrix Link';

  // The default description for this field.
  public static $default_description = 'Provides a link to the Genotype Matrix.';

  // The default widget for this field.
  public static $default_widget = 'local__genotype_matrix_link_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__genotype_matrix_link_formatter';

  // The module that manages this field.
  // If no module manages the field (IE it's added via libraries)
  // set this to 'tripal_chado'
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

  // Indicates the download formats for this field.  The list must be the
  // name of a child class of the TripalFieldDownloader.
  public static $download_formatters = array(
     'TripalTabDownloader',
     'TripalCSVDownloader',
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The DATABASE name, as it appears in chado.db.  This also builds the link-out url.  In most cases this will simply be the CV name.  In some cases (EDAM) this will be the SUBONTOLOGY.
    'term_vocabulary' => 'local',
    // The name of the term.
    'term_name' => 'genotype_matrix_link',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'genotype_matrix_link',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asynchronously.
    'auto_attach' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => '',
    // The column of the table in Chado where the value of the field comes from.
    'chado_column' => '',
    // The base table.
    'base_table' => '',
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;

  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick search field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = TRUE;

 /**
  * @see ChadoField::load()
  *
  **/
  public function load($entity) {

    // THIS FIELD MUST CONTAIN DATA IN ORDER FOR THE FORMATTER TO BE RENDERED.
    $field_name = $this->instance['field_name'];

    // schema:url for the path to link to.
    // taxrank::genus for the genus
    // NCIT:C47885 for the project name
    // CO_010:0000044 (accession) for the germplasm names
    // SO:sequence_variant for the variant names

    // @debug dpm($entity, 'entity');
    // Determine the type of page we're on.
    // And set genus/filter criteria based on that.
    $value = array();
    // to be used internally. all values must also be stored in $values.
    $qparams = array();
    if ($entity->chado_record->type_id->name == 'sequence_variant') {
      // Sequence variant name.
      $value['SO:sequence_variant'] = array($entity->chado_record->name);
      $value['TAXRANK:genus'] = $entity->chado_record->organism_id->genus;
      $qparams['variant_name'] = $value['SO:sequence_variant'];
    }
    if ($entity->chado_record->type_id->name == 'genetic_marker') {
      // Sequence variant name.
      // We access this through the relationships
      // Specifically, by looking for a related entity of type sequence_variant.
      if (isset($entity->sbo__relationship)) {
        foreach ($entity->sbo__relationship['und'] as $rel) {
          if ($rel['value']['local:relationship_subject']['rdfs:type'] == 'sequence_variant') {
            $value['SO:sequence_variant'][] = $rel['value']['local:relationship_subject']['schema:name'];
          }
          elseif ($rel['value']['local:relationship_object']['rdfs:type'] == 'sequence_variant') {
            $value['SO:sequence_variant'][] = $rel['value']['local:relationship_object']['schema:name'];
          }
        }
      }
      $value['TAXRANK:genus'] = $entity->chado_record->organism_id->genus;
      $qparams['variant_name'] = $value['SO:sequence_variant'];
    }
    elseif ($entity->chado_table == 'project') {
      // Chado Project ID.
      $value['NCIT:C47885'] = array($entity->chado_record_id);
      $genus = chado_query(
        "SELECT value FROM {projectprop} WHERE project_id=:id
          AND type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='genus')",
        array(':id' => $entity->chado_record_id))->fetchField();
      $value['TAXRANK:genus'] = (!empty($genus)) ? $genus : NULL;
      $qparams['project_id'] = $value['NCIT:C47885'];
    }

    // Assuming we have a genus, build the URL.
    if ($value['TAXRANK:genus'] !== NULL) {
      $value['schema:url'] = '/chado/genotype/' . $value['TAXRANK:genus'];
      $entity->{$field_name}['und'][0]['value'] = $value;
      $entity->{$field_name}['und'][0]['qparams'] = $qparams;
    }
  }



  /**
  * @see ChadoField::query()
  *
  **/
  public function query($query, $condition) {
  }

  /**
  * @see ChadoField::queryOrder()
  *
  **/
  public function queryOrder($query, $order) {
  }


  /**
  * @see ChadoField::elementInfo()
  *
  **/
  public function elementInfo() {
    $field_term = $this->getFieldTermID();
    return array(
      $field_term => array(
        'operations' => array('eq', 'ne', 'contains', 'starts'),
        'sortable' => TRUE,
        'searchable' => TRUE,
      ),
    );
  }

}
