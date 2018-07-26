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
    'title' => 'Variant Marked-up Sequence (FASTA format)',
    'description' => 'The following FASTA record shows the flanking sequence for this [rdfs__type] <strong>including IUPAC codes for any other variants falling within this region</strong>.',
    'show_blank_fasta' => TRUE,
    'blank_fasta_title' => 'Flanking Sequence (FASTA format)',
    'blank_fasta_desc' => 'The following FASTA record shows the flanking sequence for this [rdfs__type] <strong>without any variants taken into account</strong>.',
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

    // Grab the parent settings form in all it's glory.
    $element = parent::settingsForm($view_mode, $form, $form_state);

    // Retrieve the current settings not handled by our parent.
    $settings = $this->instance['display'][$view_mode]['settings'];

    $element['title'] = array(
      '#type' => 'textfield',
      '#title' => 'Marked-up Sequence Record: Title',
      '#description' => 'The title for the section containing the marked-up sequence fasta record.',
      '#default_value' => $settings['title'],
    );

    $element['description'] = array(
      '#type' => 'textarea',
      '#title' => 'Marked-up Sequence Record: Description',
      '#description' => 'A helpful description for the section containing the marked-up sequence fasta record.',
      '#default_value' => $settings['description'],
    );

    $element['show_blank_fasta'] = array(
      '#type' => 'checkbox',
      '#title' => 'Show Flanking Sequence Record',
      '#description' => 'If checked, the simple flanking sequence fasta record will be shown in addition to the marked-up fasta record.',
      '#default_value' => $settings['show_blank_fasta'],
    );

    $element['blank_fasta_title'] = array(
      '#type' => 'textfield',
      '#title' => 'Flanking Sequence Record: Title',
      '#description' => 'The title for the section containing the simple flanking sequence fasta record.',
      '#default_value' => $settings['blank_fasta_title'],
    );

    $element['blank_fasta_desc'] = array(
      '#type' => 'textarea',
      '#title' => 'Flanking Sequence Record: Description',
      '#description' => 'A helpful description for the section containing the simple flanking sequence fasta record.',
      '#default_value' => $settings['blank_fasta_desc'],
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

    // Ensure settings have defaults set.
    foreach (self::$default_settings as $k => $default) {
      if (!isset($settings[$k])) { $settings[$k] = $default; }

      // Check if there's a field token & if so, replace it.
      $matches = array();
      if (preg_match('/\[(\w+)\]/', $settings[$k], $matches)) {
        $key = '[' . $matches[1] . ']';
        $values = field_get_items($entity_type, $entity, $matches[1]);
        if ($values === FALSE) {
          tripal_report_error('trpfancy_fields', TRIPAL_WARNING,
            'Marked-up Sequence Field %name: %token not found, replaced with empty string.',
            array('$name' => $this->instance['field_name'], '%token' => $key));
        }

        foreach ($values as $v) {
          $value = $v['value'];
          $settings[$k] = str_replace($key, $value, $settings[$k]);
        }
      }
    }

    // If this variant/marker has anything to show...
    if (!empty($items[0]['value'])) {

      // Add CSS.
      drupal_add_css('
        #tripal_feature-fasta-record {
          font-family: Consolas, monaco, monospace;
          padding: 0px 15px;
        }
        #tripal_feature-sequence-residues {
          width: 420px;
          word-wrap: break-word;
          letter-spacing:3px;
          line-height: 18px;
          padding-top: 0;
          margin-top: 0;
          background-color: transparent;
          text-align: justify;
          font-size: 12px;
        }

        /* Make sure the SNPs are noticeable */
        .variant-marked-up-sequence .variant {
          font-weight: bolder;
          font-size: 14px;
          letter-spacing: 2px;
          color: navy;
        }
        .variant-marked-up-sequence .variant-expanded {
          color: blue;
        }', 'inline');

      // Each item is a sequence based on a location for the current variant/marker.
      foreach ($items as $key => $item) {
        $value = $item['value'];

        // First determine the FASTA header.
        $fasta_header = strtr(
          '&gt@backbone:@min-@max (@type: @coords)',
          array(
            '@backbone' => $value['data:3002'],
            '@min' => $value['local:fmin'],
            '@max' => $value['local:fmax'],
            '@type' => 'SNP',
            '@coords' => $value['local:variant_coordinates'],
          )
        );

        // Second, further mark-up the sequence.
        // Specifically, we have a sequence with the alleles substituted but would
        // further like to colour code those alleles and link to their variants.
        // We have a lot of additional information passed in as sequence variants
        // (SO:0001060), which can be added in using the index (SIO:000056).
        $sequence_with_variants = $value['local:sequence_with_variants'];
        $seq_array = array();
        $in_brackets = FALSE;
        $i = 0;
        // However, we have to be careful since the index considers [A/T/C] as one
        // character... split with caution!
        foreach (str_split($sequence_with_variants) as $char) {

          $seq_array[$i] = (isset($seq_array[$i])) ? $seq_array[$i] . $char : $char;

          if ($char == '[') {
            $in_brackets = TRUE;
          }
          elseif ($char == ']') {
            $in_brackets = FALSE;
            $i++;
          }
          elseif ($in_brackets === TRUE) {
            // Do not iterate $i b/c we want them to collapse into the same cell.
          }
          else {
            $i++;
          }
        }

        // Now we can add information using the index (SIO:000056).
        foreach ($value['SO:0001060'] as $variant) {
          if (isset($variant['SIO:000056'])) {
            $index = $variant['SIO:000056'];
            if (isset($seq_array[ $index ])) {

              // Determine classes based on if this is an IUPAC code or not.
              $classes = array('variant');
              if (strlen($variant['SO:0001023']) == 1) {
                $classes[] = 'variant-IUPAC';
                $classes[] = 'variant-' . $variant['SO:0001023'];
              }
              else {
                $classes[] = 'variant-expanded';
                $classes[] = 'current';
              }

              // Expand the current index with the added information.
              $seq_array[ $index ] = strtr(
                // @todo allow this to be configured!
                '<span class="@classes" title="@label">@allele</span>',
                array(
                  '@classes' => implode(' ', $classes),
                  '@label' => $variant['rdfs:label'],
                  '@allele' => $variant['SO:0001023'],
                )
              );
            }
          }
        }

        // Finally, recombine for display :-).
        $sequence_with_variants = implode('', $seq_array);

        $element[$key] = array(
          '#type' => 'markup',
          '#markup' => '
            <h4>'.$settings['title'].'</h4>
            <p>'.$settings['description'].'</p>
            <div id="tripal_feature-fasta-record">
              <div id="tripal_feature-sequence-header">'.$fasta_header.'</div>
              <div id="tripal_feature-sequence-residues" class="variant-marked-up-sequence">
                '.$sequence_with_variants.'
              </div>
            </div>
            <br />',
        );

        if ($settings['show_blank_fasta']) {
          $element[$key]['#markup'] .= '
            <h4>'.$settings['blank_fasta_title'].'</h4>
            <p>'.$settings['blank_fasta_desc'].'</p>
            <div id="tripal_feature-fasta-record">
              <div id="tripal_feature-sequence-header">'.$fasta_header.'</div>
              <div id="tripal_feature-sequence-residues" class="variant-marked-up-sequence">
                '.$value['data:2044'].'
              </div>
            </div>';
        }
      }
    }
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

    $settings = $this->instance['display'][$view_mode]['settings'];

    // Ensure settings have defaults set.
    foreach (self::$default_settings as $k => $default) {
      if (!isset($settings[$k])) { $settings[$k] = $default; }
    }

    return '<strong>Title:</strong> '.$settings['title'];
  }

}
