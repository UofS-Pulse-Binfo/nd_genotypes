<?php
/**
 * @class
 * Purpose: Display flanking sequence for the current variant showing all
 *  known flanking variants.
 *
 * Data:
 *   1. Sequence Data: Each variant is expected to be located on a feature with
 *      sequence via the featureloc table. The sequence should be in the
 *      feature.residues field of the srcfeature.
 *   2. Variant Information: Genotypic data is expected to be stored in the
 *      genotype_call table created by this module. Allele Calls should be
 *      saved in the bi-allelic form (e.g. AA vs. A) and only SNPs are
 *      supported.
 *
 * Assumptions:
 *   1. Only SNPs are supported, all other types of variation are excluded.
 *   2. Bi-allelic SNPs are decomposed in order to determine IUPAC code. For
 *      example, if you have AA, AG, GG as alleles, the IUPAC code choosen
 *      will be R (A + G = R).
 */
class local__sequence_with_variants extends ChadoField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Sequence';

  // The default description for this field.
  public static $default_description = 'Display flanking sequence for the current variant showing all known flanking variants.';

  // The default widget for this field.
  public static $default_widget = 'local__sequence_with_variants_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__sequence_with_variants_formatter';

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
    'term_name' => 'sequence_with_variants',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'sequence_with_variants',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should ahve auto_attach set to FALSE so tha their values can be
    // attached asyncronously.
    'auto_attach' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'feature',
    // The column of the table in Chado where the value of the field comes from.
    'chado_column' => 'feature_id',
    // The base table.
    'base_table' => 'feature',

    // Whether the field is attached to a variant or a marker.
    'is_marker' => TRUE,
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
   *
   *
   */
  public function load($entity) {
    // Placeholder for the value of this field.
    // Populating this is just cleaner code-wise then populating the entity
    // directly. At the end this will be assigned to
    // $entity->local__sequence_with_variants['und'][0]['value']
    $value = array();

    // ChadoFields automatically load the chado column specified in the
    // default settings above. If that is all you need then you don't even
    // need to implement this function. However, if you need to add any
    // additional data to be used in the display, you should add it here.
    parent::load($entity);

    // Retrieve the settings.
    $settings = $this->instance['settings'];

    // First, retrieve the chado feature_id for the current Tripal Content.
    $feature_id = $entity->local__sequence_with_variants['und'][0]['chado-feature__feature_id'];

    // Ensure we have a $variant_id and $marker_id set based on what we are attached to.
    //  - Attached to a Variant.
    if ($settings['is_marker'] == FALSE) {
      $variant_id = $feature_id;
      $marker_id = NULL;
      $category = 'variant';
    }
    //  - Attached to a marker.
    else {
      $marker_id = $feature_id;
      $variant_id = nd_genotypes_get_variant($marker_id, TRUE);
      $category = 'marker';
    }

    // Determine the partition.
    if (isset($entity->chado_record->organism_id)) {
      $partition = strtolower($entity->chado_record->organism_id->genus);
    }
    else {
      tripal_report_error(
        'nd_genotypes',
        TRIPAL_WARNING,
        '"Sequence with Variants" field was unable to determine what the organism for entity :id is.',
        array( ':id' => $entity->id, )
      );
      $entity->local__sequence_with_variants['und'][0]['value'] = FALSE;
      return FALSE;
    }

    // First get a list of feature locations where the parent has sequence.
    $locations = chado_query('
      SELECT l.featureloc_id, l.srcfeature_id, p.name as backbone_name, l.fmin, l.fmax, l.locgroup, l.strand as strand
      FROM {featureloc} l
      LEFT JOIN {feature} p ON p.feature_id=l.srcfeature_id
      WHERE l.feature_id=:id AND p.residues IS NOT NULL
      ORDER BY locgroup ASC, p.name ASC',
      array(':id' => $variant_id))->fetchAll();
    // If there are no locations on parents with sequence then unfortunalty
    // we cannot provide this display...
    if (!$locations) {
      $entity->local__sequence_with_variants['und'][0]['value'] = FALSE;
      return FALSE;
    }

    // Now, for each location...
    $value['hydra:member'] = array();
    foreach ($locations as $k => $loc) {

      // Save the current location to the value of this field using the same format
      // as is used for the sequence coordinates field.
      $value['hydra:member'][$k]['data:3002'] = $loc->backbone_name;
      $value['hydra:member'][$k]['local:variant_coordinates'] = $loc->backbone_name . ':' . ($loc->fmin + 1);
      $value['hydra:member'][$k]['local:fmin'] = $loc->fmin + 1;
      $value['hydra:member'][$k]['local:fmax'] = $loc->fmax;
      $value['hydra:member'][$k]['SO:0000983'] = $loc->strand;

      // Retrieve the information including the sequence substring for the parent
      // feature.
      if ($loc->strand == -1) {
        // @todo See Issue #
        $srcfeature = NULL;
      }
      else {
        $srcfeature = chado_query('
          SELECT
            f.feature_id,
            f.name,
            f.uniquename,
            (fmin::int-250) as substring_start,
            substring(residues from fmin::int+1-250 for 500) as residues
          FROM {featureloc} fl
            LEFT JOIN {feature} f ON f.feature_id=fl.srcfeature_id
          WHERE fl.feature_id=:id AND fl.srcfeature_id=:parent',
        array(':id' => $variant_id, ':parent' => $loc->srcfeature_id))->fetchObject();
      }

      // We can only provide a display if we have a sequence...
      if (!$srcfeature OR empty($srcfeature->residues)) {
        if ($loc->strand != -1) {
          unset($value['hydra:member'][$k]);
        }
        else {
          $value['hydra:member'][$k]['data:2044'] = NULL;
          $value['hydra:member'][$k]['local:sequence_with_variants'] = NULL;
          $value['hydra:member'][$k]['SO:0001060'] = array();
        }
      }
      else {

        // Do some checks on the sequence to handle "edge" (literally ;-)) cases.
        // Specifically, we want to handle the case where the current variant/marker
        // is within 250 residues of the start of the parent feature. In this case
        // we won't be able to return 500 residues and the start position will need
        // to be updated accordingly.
        if ($srcfeature->substring_start < 0) {
          $srcfeature->substring_start = 0;
        }

        // Update the range to match the sequence rather then the variant.
        $value['hydra:member'][$k]['local:fmin'] = (int) $srcfeature->substring_start;
        $value['hydra:member'][$k]['local:fmax'] = $srcfeature->substring_start + strlen($srcfeature->residues) -1;
        $value['hydra:member'][$k]['data:2044'] = $srcfeature->residues;

        // Add the marked-up sequence ;-P
        $info = array(
          'srcfeature_id' => $srcfeature->feature_id,
          'srcfeature_name' => $srcfeature->name,
          'variant_id' => $variant_id,
          'sequence' => $srcfeature->residues,
          'sequence_start' => $value['hydra:member'][$k]['local:fmin'],
          'sequence_end' => $value['hydra:member'][$k]['local:fmax'],
          'sequence_strand' => $loc->strand, // assume the positive strand.
        );
        $value['hydra:member'][$k]['local:sequence_with_variants'] = nd_genotypes_markup_sequence_with_variants(
          $info,
          array(),
          $partition
        );

        // And process the variant list to provide to the formatter/ws.
        $value['hydra:member'][$k]['SO:0001060'] = array();
        foreach ($info['variants'] as $variant) {
          $value['hydra:member'][$k]['SO:0001060'][] = array(
            'SO:0001023' => $variant['code'],
            'SIO:000056' => $variant['index'],
            'rdfs:label' => $variant['tooltip'],
          );
        }
      }
    }

    // Finally assign the value to the entity.
    $entity->local__sequence_with_variants['und'][0]['value'] = $value;
    //@debug dpm($value, 'value in load');
  }

  /**
   * Provides a form for the 'Field Settings' of an instance of this field.
   *
   * This function corresponds to the hook_field_instance_settings_form()
   * function of the Drupal Field API.
   *
   * Validation of the instance settings form is not supported by Drupal, but
   * the TripalField class does provide a mechanism for supporting validation.
   * To allow for validation of your setting form you must call the parent
   * in your child class:
   *
   * @code
   *   $element = parent::instanceSettingsForm();
   * @endcode
   *
   * Please note, the form generated with this function does not easily
   * support AJAX calls in the same way that other Drupal forms do.  If you
   * need to use AJAX you must manually alter the $form in your ajax call.
   * The typical way to handle updating the form via an AJAX call is to make
   * the changes in the form function itself but that doesn't work here.
   */
  public function instanceSettingsForm() {

    // Retrieve the current settings.
    // If this field was just created these will contain the default values.
    $settings = $this->instance['settings'];

    // Allow the parent Tripal Field to set up the form element for us.
    $element = parent::instanceSettingsForm();

    $element['is_marker'] = array(
      '#type' => 'radios',
      '#title' => 'Type of Content',
      '#options' => array(
        TRUE => 'Marker: genotypes are attached directly',
        FALSE => 'Variant: related to a marker(s) with genotypic data.',
      ),
      '#default_value' => $settings['is_marker'],
    );

    return $element;
  }
}
