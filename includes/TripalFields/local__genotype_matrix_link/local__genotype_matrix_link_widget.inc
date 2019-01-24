<?php
/**
 * @class
 * Purpose:
 *
 * Allowing edit?
 * Data:
 * Assumptions:
 */
class local__genotype_matrix_link_widget extends ChadoFieldWidget {

  // The default label for this field.
  public static $default_label = 'Genotype Matrix Link';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('local__genotype_matrix_link');


 /**
  * @see ChadoFieldWidget::form()
  *
  **/

  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);
  }

  /**
  * @see ChadoFieldWidget::validate()
  *
  **/
  public function validate($element, $form, &$form_state, $langcode, $delta) {
  }

   /**
  * @see ChadoFieldWidget::submit()
  *
  **/
  public function submit($form, &$form_state, $entity_type, $entity, $langcode, $delta) {
  }

}