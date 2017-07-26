<?php
/**
 * @class
 * Purpose: Display flanking sequence for the current variant showing all
 *  known flanking variants.
 *
 * Display:
 *   This field displays a portion of the sequence flanking the current
 *   variant/marker as a FASTA record to facillitate marker design. The current
 *   variant/marker is displayed in expanded format (e.g. [A/T] ) and all
 *   variants in the flanking sequence are represented using their IUPAC codes.
 *
 *   If the current variant is located on more then one feature with sequence
 *   information then multiple FASTA records will be shown. Specifically,
 *   one per location.
 *
 * Configuration:
 *  A) In Field Configuration:
 *    - Is the current Content Type a Variant or Marker?
 *    - How to display multiple locations:
 *       1) list of FASTA records,
 *       2) vertical tabbed display,
 *       3) accordion display.
 *  B) Module Configuration:
 *    - Colours of SNP alleles can be set at the module level for consistency
 *      across fields.
 *    - Colours of non-SNP alleles are set in the same location as for SNPs but
 *      as a rotating colour pallet rather then exact assignment.
 *    ** See Tripal > Extensions > Natural Diversity Genotypes > Settings **
 */
class local__sequence_with_variants_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Variant Marked-up Sequence';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__sequence_with_variants');

  // The list of default settings for this formatter.
  public static $default_settings = array(
    //'key' => 'default_value',
  );

  /**
   * Provides the field's setting form.
   *
   * This function corresponds to the hook_field_formatter_settings_form()
   * function of the Drupal Field API.
   *
   * The settings form appears on the 'Manage Display' page of the content
   * type administration page. This function provides the form that will
   * appear on that page.
   *
   * To add a validate function, please create a static function in the
   * implementing class, and indicate that this function should be used
   * in the form array that is returned by this function.
   *
   * This form will not be displayed if the formatter_settings_summary()
   * function does not return anything.
   *
   * param $field
   *   The field structure being configured.
   * param $instance
   *   The instance structure being configured.
   * param $view_mode
   *   The view mode being configured.
   * param $form
   *   The (entire) configuration form array, which will usually have no use
   *   here.  Typically for reference only.
   * param $form_state
   *   The form state of the (entire) configuration form.
   *
   * @return
   *   A Drupal Form array containing the settings form for this field.
   */
  public function settingsForm($view_mode, $form, &$form_state) {
  }

  /**
   *  Provides the display for a field
   *
   * This function corresponds to the hook_field_formatter_view()
   * function of the Drupal Field API.
   *
   *  This function provides the display for a field when it is viewed on
   *  the web page.  The content returned by the formatter should only include
   *  what is present in the $items[$delta]['values] array. This way, the
   *  contents that are displayed on the page, via webservices and downloaded
   *  into a CSV file will always be identical.  The view need not show all
   *  of the data in the 'values' array.
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return
   *    An element array compatible with that returned by the
   *    hook_field_formatter_view() function.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    // Get the settings
    $settings = $display['settings'];

  }

  /**
   * Provides a summary of the formatter settings.
   *
   * This function corresponds to the hook_field_formatter_settings_summary()
   * function of the Drupal Field API.
   *
   * On the 'Manage Display' page of the content type administration page,
   * fields are allowed to provide a settings form.  This settings form can
   * be used to allow the site admin to define how the field should be
   * formatted.  The settings are then available for the formatter()
   * function of this class.  This function provides a text-based description
   * of the settings for the site developer to see.  It appears on the manage
   * display page inline with the field.  A field must always return a
   * value in this function if the settings form gear button is to appear.
   *
   * See the hook_field_formatter_settings_summary() function for more
   * information.
   *
   * @param $field
   * @param $instance
   * @param $view_mode
   *
   * @return string
   *   A string that provides a very brief summary of the field settings
   *   to the user.
   *
   */
  public function settingsSummary($view_mode) {
  }

}
