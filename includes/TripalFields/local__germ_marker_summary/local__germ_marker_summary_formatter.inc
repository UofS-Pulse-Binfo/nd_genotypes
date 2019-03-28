<?php
/**
 * @class
 * Purpose: Provide a quick search on entity pages which submits/redirects to a full search.
 *
 * Display: A simple textfield search form.
 * Configuration:
 *   - path to the full search.
 *   - the URL token (query parameter) the value applies to.
 *   - help text.
 *   - textfield placeholder.
 *   - search button text.
 *   - autocomplete path.
 */
class local__germ_marker_summary_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Germplasm Marker Summary';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__germ_marker_summary');

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
    // Grab the name of the field to create a unique ID for the chart.
    $field_name = $this->instance['field_name'];
    $display_vars = $entity->{$field_name}['und'][0]['vars'];

    // Style information.
    drupal_add_css(drupal_get_path('module', 'nd_genotypes') . '/includes/TripalFields/local__germ_marker_summary/theme/style_field.css');

    $genotype_matrix_link = l(t('Go to Genotype Matrix'), $display_vars['genotype_matrix']);

    // Add element:
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => '
        <div id="field-germ-marker-summary-wrapper">
          <div><h1>Makers: ' . $display_vars['summary_count']  . '</h1>&nbsp; | &nbsp;'. $genotype_matrix_link . '</div>
        </div>
      '
    );

    return $element;
  }
}
