<?php
/**
 * @class
 * Purpose:
 *
 * Display:
 * Configuration:
 */
class so__genetic_marker_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Genetic Marker List';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('so__genetic_marker');

  // The list of default settings for this formatter.
  public static $default_settings = array(
    'setting1' => 'default_value',
  );

  /**
   * @see ChadoFieldFormatter::settingsForm()
   *
   */
  public function settingsForm($view_mode, $form, &$form_state) {

  }

  /**
   * @see ChadoFieldFormatter::View()
   *
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    // Get the settings
    $settings = $display['settings'];

    // Compile our list based on the items.
    $list = array();
    foreach ($items as $item) {
      if ($item['values']['schema:url']) {
        $list[] = l($item['values']['schema:name'], $item['values']['schema:url']);
      }
      else {
        $list = $item['values']['schema:name'];
      }
    }

    // Display our list with comma's separating items.
    $element[0] = array(
      '#type' => 'markup',
      '#markup' => implode(', ',$list),
    );
  }

  /**
   * @see ChadoFieldFormatter::settingsSummary()
   *
   */
  public function settingsSummary($view_mode) {
    return '';
  }

}
