<?php
/**
 * @class
 * Purpose: Provides a summary of genotypic data for a given variant.
 *
 * Display:
 *   This widget uses D3.js to draw a pie chart summarizing the ratio of genotypic
 *   calls observed for a given variant. If more then one marker assays this
 *   variant, then you will see one donut/ring per marker with the ratio of calls
 *   per marker being displayed within that donut/ring.
 *
 * Value:
 *   This formatter expects to recieve data as follows:
 *    - Each marker attached to the current variant should be indicated as
 *      it's own item (ie: $entity->local__variant_genotype_summary['und'][0]
 *      should contain all details about a given marker).
 *    - Within the 'value' of a given marker there should be the following keys:
 *        - rdfs:label = the label to give to the donut/ring
 *        - hydra:member = each element of this array should be a piece of the pie
 *            and should contain a rdfs:label and local:count to be used to label
 *            and size the pie respectively.
 *
 * Configuration:
 *  A) In Field Configuration:
 *    - Figure Title: This is the title of the figure shown to the user. It is
 *      located directly under the pie chart (bolded) and briefly describes the
 *      pie chart.
 *    - Figure Legend: Follows the title and provides additional description to
 *      the user about the pie chart.
 *    ** Defaults can be found in $default_settings below. **
 *  B) Module Configuration:
 *    - Colours of SNP alleles can be set at the module level for consistency
 *      across fields.
 *    - Colours of non-SNP alleles are set in the same location as for SNPs but
 *      as a rotating colour pallet rather then exact assignment.
 *    ** See Tripal > Extensions > Natural Diversity Genotypes > Settings **
 */
class local__variant_genotype_summary_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Genotype Pie Chart';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__variant_genotype_summary');

  // The list of default settings for this formatter.
  public static $default_settings = array(
    'title' => 'The ratio of alleles per marker assaying this variant.',
    'legend' => 'The current variant has been assayed by :num_markers different marker(s). The ratio of alleles for each marker is shown as one ring composing the pie chart. This allows you to compare the ratio across marker(s), as well as, get an overall idea of the ratio of alleles.',
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
    $element = array();

    $element['title'] = array(
      '#type' => 'textfield',
      '#title' => 'Figure Title',
      '#default_value' => self::$default_settings['title'],
      '#description' => 'A breif title for the figure.',
    );

    $element['legend'] = array(
      '#type' => 'textarea',
      '#title' => 'Figure Legend',
      '#default_value' => self::$default_settings['legend'],
      '#description' => 'Should describe the figure to the user.',
    );

    return $element;
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
    // Fill in defaults for any settings not set.
    foreach (self::$default_settings as $key => $value) {
      if (!isset($settings[$key])) {
        $settings[$key] = $value;
      }
    }

    // If this variant has any genotypes attached to it...
    if (!empty($items[0]['value'])) {

      // Process items to expose to JS.
      $data = array();
      $js_settings = array();
      nd_genotypes_set_default_colours();
      $colours = unserialize(variable_get('nd_genotypes_SNP_colours'));
      $extra_colours = unserialize(variable_get('nd_genotypes_allele_colours'));
      foreach ($items as $key => $value) {
        $data[$key] = array();
        $data[$key]['label'] = $value['value']['rdfs:label'];
        $data[$key]['parts'] = array();
        foreach ($value['value']['hydra:member'] as $part) {
          $data[$key]['parts'][] = array(
            'label' => $part['rdfs:label'],
            'count' => $part['local:count'],
          );

          // Also add colours as we see them.
          if (isset($colours[ $part['SO:0001023'] ])) {
            $js_settings['SNPcolours'][ $part['SO:0001023'] ] = $colours[ $part['SO:0001023'] ];
          }
          else {
            $js_settings['SNPcolours'][ $part['SO:0001023'] ] = array_shift($extra_colours);
          }
        }
      }

      // Expose the data & settings to JS.
      drupal_add_js(array('nd_genotypes' => array('genotypes' => $data)), 'setting');
      drupal_add_js(array('nd_genotypes' => array('settings' => $js_settings)), 'setting');

      // Add D3.js and our Pie chart javascript.
      drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js');
      drupal_add_js(drupal_get_path('module','nd_genotypes').'/theme/js/genotypeFeatureMultiPieChart.js');

      // Add CSS
      drupal_add_css('
        #nd-genotypes-pie-chart {
          padding: 50px 20px 20px 20px;
        }
        #nd-genotypes-summary .legend-title{
          font-weight: bold;
        }', 'inline');

      // Used in the figure legend.
      $tokens = array(
        ':num_markers' => sizeof($items),
      );

      // Finally, add the markup.
      $element[0] = array(
        '#type' => 'markup',
        '#markup' => '
          <div id="nd-genotypes-summary" class="figure">
            <div id="nd-genotypes-pie-chart"></div>
            <div class="legend">
              <span class="legend-title">Figure: '.strtr($settings['title'], $tokens).'</span>
              <span class="legend-description">'.strtr($settings['legend'], $tokens).'</span>
          </div>
        </div>',
      );
    }

    return $element;
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
    return 'Genotype Pie Chart';
  }

}
