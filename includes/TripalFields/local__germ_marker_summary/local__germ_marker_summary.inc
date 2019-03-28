<?php
/**
 * @class
 * Purpose: Provide a graphical summary of data stored in a materialized view.
 *   This is a generic, configurable fields to make it easier to add charts
 *   to Tripal Content pages.
 *
 * Data:
 *   Data is pulled from a configured materialized view and filtered based on a
 *   specific Tripal/Chado field.
 * Assumptions:
 *   - Currently there can only be one such chart per page. If you need more then
 *     one chart, a current work around is to extend this class using a different
 *     term.
 */
class local__germ_marker_summary extends ChadoChartField {

  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Germplasm Marker Summary';

  // The default description for this field.
  public static $default_description = 'Germplasm Marker Summary';

  // The default widget for this field.
  public static $default_widget = 'local__germ_marker_summary_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__germ_marker_summary_formatter';

  // The module that manages this field.
  public static $module = 'nd_genotypes';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'local',
    // The name of the term.
    'term_name' => 'germ_marker_summary',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'germ_marker_summary',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The format for display of the organism.
    'field_display_string' => '',
  );

  /// A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick search field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = FALSE;


  // ChadoFields automatically load the chado column specified in the
  // default settings above. If that is all you need then you don't even
  // need to implement this function. However, if you need to add any
  // additional data to be used in the display, you should add it here.
  public function load($entity) {
    $germplasm = array(
      // Chado record id coincides with stock.stock_id number
      'stock_id' => $entity->chado_record_id,
      // Genus
      'genus' => $entity->chado_record->organism_id->genus,
    );

    // We need the field name to ensure we save the data in the correct field!
    $field_name = $this->instance['field_name'];

    // Set this property for formatteer to render.
    $entity->{$field_name}['und'][0]['value']['local:germ_marker_summary'] = '';

    // Save
    $entity->{$field_name}['und'][0]['vars'] = array();

    // This field will output a summary marker count of a germplasm.
    // Markers : Summary count eg. 293,299 | Go to Genotype Matrix link.
    $genotype_matrix_link = $GLOBALS['base_url'] . '/chado/genotype/' . ucfirst($germplasm['genus']) . '?germplasm[0]=' . $germplasm['stock_id'];

    $summary_count = 0;

    if (function_exists('ndg_mview_query')) {
      // The genus of this germplasm. Use in identifying the partition in the function below.
      $partition =  strtolower(trim($germplasm['genus']));

      // Count markers for this germplasm.
      $sql = sprintf("
        SELECT COUNT(DISTINCT marker_id) AS summary_count
        FROM {mview_ndg_calls} WHERE stock_id = %d OR germplasm_id = %d
        LIMIT 1", $germplasm['stock_id'], $germplasm['stock_id']);

      // Query materialized view, using the sql and partion.
      $result = ndg_mview_query($partition, $sql);

      if ($result) {
        // Summary count of this stock.
        $s = $result->fetchField();
        $summary_count = number_format($s);
      }
    }

    // Save
    $entity->{$field_name}['und'][0]['vars'] = array(
      'summary_count' => $summary_count,
      'genotype_matrix' => $genotype_matrix_link,
    );
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


    return $element;
  }

  /**
   *
   */
  public function instanceSettingsFormValidate($form, &$form_state) {

    parent::instanceSettingsFormValidate($form, $form_state);
    $values = $form_state['values']['instance']['settings']['data_options'];

  }
}
