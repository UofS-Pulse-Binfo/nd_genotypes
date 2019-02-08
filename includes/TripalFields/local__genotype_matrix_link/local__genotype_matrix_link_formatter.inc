<?php
/**
 * @class
 * Purpose:
 *
 * Display:
 * Configuration:
 */
class local__genotype_matrix_link_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Genotype Matrix Link';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__genotype_matrix_link');

  // The list of default settings for this formatter.
  public static $default_settings = array(
    'help_text' => 'You can access the genotypic data through the genotypic matrix. By clicking the link below, you will be redirected to the genotype matrix with filter criteria filled in based on the page you are on. Keep in mind at a minimum, germplasm needs to be supplied in order to access the data.',
    'button_text' => 'Filtered Genotype Matrix',
  );

 /**
  * @see ChadoFieldFormatter::settingsForm()
  *
  **/
  public function settingsForm($view_mode, $form, &$form_state) {

  }

 /**
  * @see ChadoFieldFormatter::View()
  *
  **/
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    // Retrieve the current settings not handled by our parent.
    $settings = $this->instance['display'][$view_mode]['settings'];
    $our_setting_keys = array('help_text', 'button_text');
    foreach ($our_setting_keys as $k) {
      if (!isset($settings[$k])) { $settings[$k] = self::$default_settings[$k]; }
    }

    if ($items[0]['value']['TAXRANK:genus']) {

      // Add our CSS.
      $module_path = drupal_get_path('module', 'nd_genotypes');
      $element['#attached']['css'][] = $module_path . '/includes/TripalFields/local__genotype_matrix_link/theme/local__genotype_matrix_link.css';

      // Use #tree to support multiple sub-elements.
      $element[0] = array(
        '#tree' => TRUE,
      );

      // Matrix Thumbnail.
      $img_path = url($module_path . '/includes/TripalFields/local__genotype_matrix_link/theme/GenotypeMatrixThumbnail.png');
      $element[0][0] = array(
        '#type' => 'markup',
        '#prefix' => '<span class="thumbnail">',
        '#markup' =>  '<img src="'.$img_path.'" alt="Genotype Matrix Thumbnail">' ,
        '#suffix' => '</span>',
      );

      // Help text.
      $element[0][1] = array(
        '#type' => 'markup',
        '#markup' => '<p class="help-text">' . $settings['help_text'] . '</p>',
      );

      // Link.
      $element[0][2] = array(
        '#type' => 'markup',
        '#prefix' => '<div class="nd-button">',
        '#markup' => l(
          $settings['button_text'],
          $items[0]['value']['schema:url'],
          array('query' => $items[0]['qparams'])
        ),
        '#suffix' => '</div>',
      );
    }
  }

 /**
  * @see ChadoFieldFormatter::settingsSummary()
  *
  **/
  public function settingsSummary($view_mode) {
    return '';
  }

}
