<?php
/**
 * @file
 * Provides specialized form elements for this module.
 */

/**
 * Register KP Genotype-specific form elements
 */
function nd_genotypes_element_info() {
  $elements = array();

  $elements['multiple_organism_stock'] = array(
    '#input' => TRUE,
    '#process' => array('expand_multiple_organism_stock'),
    '#value_callback' => 'multiple_organism_stock_value_callback',
    '#theme' => array('multiple_organism_stock'),
    '#theme_wrappers' => array('form_element'),
    '#tree' => TRUE,
  );

  $elements['pairwise_compare_germplasm'] = array(
    '#input' => TRUE,
    '#process' => array('expand_pairwise_compare_germplasm'),
    '#element_validate' => array('pairwise_compare_germplasm_validate'),
    '#theme' => array('pairwise_compare_germplasm'),
    '#theme_wrappers' => array('form_element'),
    '#tree' => TRUE,
  );

  return $elements;
}

/**
 * Custom form element that keeps track of multiple organism_stocks (see above form element).
 * There is a list of already added stock names with the ability to remove them by selecting a
 * checkbox and an organism_stock form element to add to the list.
 *
 * Example of how to use it:
 * @code
		$form['element'] = array(
			'#type' => 'multiple_organism_stock',
			'#organism_options' => $organisms_keyed_by_id,
			'#default_value' => $form_state['values']['element'],
		);
 * @endcode
 */
function expand_multiple_organism_stock ($element) {

  $parents = $element['#parents'];
  $element['#tree'] = TRUE;
  if (isset($element['#default_value'])) $element['#value'] = $element['#default_value'];

  // Set some defaults.
  multiple_organism_stock_set_element_defaults($element);

  // We need to keep track of when the user has access to the elements in order
  // to ensure we don't validate already validated data. Our approach to this is to assume
  // that once the form element is rendered (directly after this callback) then the
  // user has access to it and it needs to be validated again. After validation
  // in the value callback the following property will be set to TRUE blocking validation
  // until the form element is rendered again. This is necessary because the value callback
  // is called 2X during the submission & rebuilding process.
  $element['#nd_validated'] = FALSE;

  if (isset($element['#value']['existing'])) {

    foreach ($element['#value']['existing'] as $k => $v) {
      $element['existing'][$k] = array(
        '#prefix' => '<span class="container-inline clearfix">',
        '#suffix' => '</span>',
      );

      $element['existing'][$k]['stock_name'] = array(
        '#type' => 'textfield',
        '#title' => $element['#name_title'],
        '#title_display' => 'invisible',
        '#default_value' => $v['stock_name'],
        //'#disabled' => TRUE,
      );
      if (isset($element['#name_autocomplete_path'])) {
        $element['existing'][$k]['stock_name']['#autocomplete_path'] = $element['#name_autocomplete_path'];
      }

      $element['existing'][$k]['organism_id'] = array(
        '#type' => 'select',
        '#title' => $element['#organism_title'],
        '#title_display' => 'invisible',
        '#options' => $element['#organism_options'],
        '#default_value' => $v['organism_id'],
        //'#disabled' => TRUE,
      );

      $element['existing'][$k]['remove-' . $k] = array(
        '#type' => 'image_button',
        '#src' => drupal_get_path('module','nd_genotypes').'/theme/images/remove-button-x-nickel.png',
        '#name' => 'remove-' . $k,
        '#value' => 'remove-' . $k,
        '#prefix' => '<span class="remove no-button">',
        '#suffix' => '</span>'
      );
      /*
      $img = theme_image(array(
        'path' => drupal_get_path('module','cross_manage').'/theme/images/remove-button-x.png',
        'title' => 'Remove this parent from the Crossing Block.',
        'alt' => 'Remove Parent',
      'attributes' => array('class' => 'remove-icon')
      ));
      $form['parents'][$pid]['action'] = array(
        '#type' => 'markup',
        '#markup' =>  l($img . 'remove', 'node/'.$parents[$i]->nid.'/edit', array('html' => TRUE, 'attributes' => array('target' => '_blank'))),
      );
      */
    }
  }

  $element['new'] = array(
    '#prefix' => '<span class="container-inline clearfix">',
    '#suffix' => '</span>',
  );

  $element['new']['stock_name'] = array(
    '#type' => 'textfield',
    '#title' => $element['#name_title'],
    '#title_display' => 'invisible',
    '#attributes' => array('placeholder' => $element['#name_title']),
    '#value' => (isset($element['#value']['new']['stock_name'])) ? $element['#value']['new']['stock_name'] : NULL,
  );

  $element['new']['organism_id'] = array(
    '#type' => 'select',
    '#options' => $element['#organism_options'],
    '#value' => (isset($element['#value']['new']['organism_id'])) ? $element['#value']['new']['organism_id'] : NULL,
  );
  if (isset($element['#name_autocomplete_path'])) {
    $element['new']['stock_name']['#autocomplete_path'] = $element['#name_autocomplete_path'];
  }

  $element['new']['add'] = array(
    '#type' => 'image_button',
    '#name' => 'add',
    '#value' => 'Add Germplasm',
    '#src' => drupal_get_path('module', 'nd_genotypes') . '/theme/images/add-button.png',
    '#prefix' => '<span class="add-more image-button green-button">',
    '#suffix' => '</span>'
  );

  $element['description'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="description">' . $element['#added_description'] . '</div>',
  );

  return $element;
}

/**
 * Set some defaults for the multiple organism stock form element.
 */
function multiple_organism_stock_set_element_defaults(&$element) {

  // Organism related options.
  if (!isset($element['#organism_title'])) $element['#organism_title'] = 'Species';
  if (!isset($element['#organism_options'])) $element['#organism_options'] = array();
  $element['#organism_options'][0] = $element['#organism_title'];

  // Stock name related options.
  if (!isset($element['#name_title'])) $element['#name_title'] = 'Stock Name';

  // Global Element options.
  if (!isset($element['#added_description'])) $element['#added_description'] = 'Specify germplasm by indicating the name and species.';

}

/**
 * Theme the custom form element
 */
function theme_multiple_organism_stock ($vars) {
  drupal_add_css(drupal_get_path('module','nd_genotypes').'/theme/css/nd_genotypes.multiple_stock_element.css');
  return '';
}

/**
 * Value callback for the multiple_organism_stock custom form element.
 *
 * This function both validates input and processes the input
 * when add/remove buttons are pressed. The validate callback was not used for validation
 * because it is called after the value callback (ie: after the data has already been
 * processed). Furthermore, the value callback provides more direct access to the input
 * entered into the form element making validation easier.
 *
 * One thing to keep in mind is that the value callback is called both when the form
 * is submitted and when the form is rendered. To avoid excessive validation, we keep
 * of whether the callback is being called on submit (we should validate) or render
 * (we've already validated since the user last had access).
 */
function multiple_organism_stock_value_callback(&$element, $input = FALSE, &$form_state) {

  // If the element has already been validated then it has also been processed.
  // As such there is no need to do so again.
  if(isset($element['#nd_validated']) AND $element['#nd_validated'] === TRUE) return $input;

  // We set this flag here even though we haven't yet validated the data in order to
  // ensure it gets set even if validation fails.
  $element['#nd_validated'] = TRUE;

  // Set some defaults.
  multiple_organism_stock_set_element_defaults($element);

  // Do some initial processing first.
  //-----------------------------------
  // Remove empty values from existing.
  if (isset($input['existing'])) {
    foreach ($input['existing'] as $k => $v) {
      if (empty($v['stock_name'])) {
        unset($input['existing'][$k]);
      }
    }
  }

  // Remove a germplasm.
  $to_remove = array_search('-', $form_state['input']);
  if ($to_remove AND preg_match('/remove-(\d+)/', $to_remove, $matches)) {
    if (isset($input['existing'][$matches[1]])) {
      unset($input['existing'][$matches[1]]);
    }
  }

  // Ok, First Validation
  //-----------------------------------

  $stub = implode('][',$element['#parents']);

  // If the add button was pressed...
  if(isset($form_state['input']['add'])) {

    // then both the name and organism should have been provided.
    if (empty($input['new']['stock_name'])) {
      form_set_error($stub . '][new][stock_name',
        t('The %subname is required when adding germplasm to the %name list.',
          array('%subname' => $element['#name_title'], '%name' => $element['#title'])));
      $element['#nd_errors'] = TRUE;
    }
    if (empty($input['new']['organism_id'])) {
      form_set_error($stub . '][new][organism_id',
        t('The %subname is required when adding germplasm to the %name list.',
          array('%subname' => $element['#organism_title'], '%name' => $element['#title'])));
      $element['#nd_errors'] = TRUE;
    }

    // Furthermore, the stock should actually exist.
    if (!empty($input['new']['stock_name']) AND !empty($input['new']['organism_id'])) {
      $stock = tripal_get_stock(array('name' => $input['new']['stock_name'], 'organism_id' => $input['new']['organism_id']));
      if (!$stock) {
        form_set_error($stub . '][new',
          t('The germplasm you attempted to add to %name does not exist or the %subname is not unique for the specified %org.',
            array('%subname' => $element['#name_title'], '%org' => $element['#organism_title'], '%name' => $element['#title'])));
        $element['#nd_errors'] = TRUE;
      }
      // If we did find a stock we might as well save the stock_id for later.
      else {
        $input['new']['stock_id'] = $stock->stock_id;
      }
    }
  }

  // Check all existing germplasm as well.
  if (isset($input['existing'])) {
    foreach ($input['existing'] as $k => $v) {

      // then both the name and organism should have been provided.
      if (empty($input['existing'][$k]['stock_name'])) {
        form_set_error($stub . '][existing]['.$k.'][stock_name',
          t('The %subname is required for germplasm in the %name list.',
            array('%subname' => $element['#name_title'], '%name' => $element['#title'])));
        $element['#nd_errors'] = TRUE;
      }
      if (empty($input['existing'][$k]['organism_id'])) {
        form_set_error($stub . '][existing]['.$k.'][organism_id',
          t('The %subname is required for germplasm in the %name list.',
            array('%subname' => $element['#organism_title'], '%name' => $element['#title'])));
        $element['#nd_errors'] = TRUE;
      }

      // Furthermore, the stock should actually exist.
      if (!empty($input['existing'][$k]['stock_name']) AND !empty($input['existing'][$k]['organism_id'])) {
        $stock = tripal_get_stock(array('name' => $input['existing'][$k]['stock_name'], 'organism_id' => $input['existing'][$k]['organism_id']));
        if (!$stock) {
          form_set_error($stub . '][existing]['.$k,
            t('The germplasm in the %name list does not exist or the %subname is not unique for the specified %org.',
              array('%subname' => $element['#name_title'], '%org' => $element['#organism_title'], '%name' => $element['#title'])));
          $element['#nd_errors'] = TRUE;
        }
        // If we did find a stock we might as well save the stock_id for later.
        else {
          $input['existing'][$k]['stock_id'] = $stock->stock_id;
        }
      }

    }
  }

  if (isset($element['#nd_errors']) AND $element['#nd_errors'] === TRUE) return $input;

  // And then finish processing the values!
  //-----------------------------------

  // Add a germplasm.
  if(isset($form_state['input']['add'])) {
    if (!isset($input['existing'])) {
      $input['existing'] = array();
    }

    $input['existing'][] = $input['new'];

    $input['new']['stock_name'] = '';
    $input['new']['organism_id'] = 0;
  }

  return $input;
}

///////////////////////////////////////////////////////////
// Form element: pairwise_compare_germplasm
///////////////////////////////////////////////////////////
/**
 * Custom form element allowing you to select two stocks to be compared from two select lists.
 *
 * Example on how to use it:
 * @code
    $options = array(
      'stock1',stock2','stock3'
    );
		$form['element'] = array(
			'#type' => 'pairwise_compare_germplasm',
			'#options' => array(
			  'first' => $options,
			  'second' => $options
			),
			'#default_value' => $form_state['values']['element'],
		);
 * @endcode
 * Will give you two select lists where each select list contains the items in $options.
 */
function expand_pairwise_compare_germplasm ($element) {

  // Assigning default values to element. You can override these values by passing your own values using "#default_value".
  if (empty($element['#value'])) {
    $element['#value'] = array(
      'first' => '<NONE>',
      'second' => '<NONE>',
    );
  }

  // Add <none> to options
  $element['#options']['first']['<NONE>'] = '';
  $element['#options']['second']['<NONE>'] = '';

  $element['#tree'] = TRUE;
  $parents = $element['#parents'];

  $parents[] = 'first';
  $element['first'] = array(
    '#type' => 'select',
    '#default_value' => $element['#value']['first'],
    '#options' => $element['#options']['first'],
    '#prefix' => '<div class="pairwise_compare_germplasm-element container-inline">Between ',
    '#validated' => TRUE
  );

  $parents[] = 'second';
  $element['second'] = array(
    '#type' => 'select',
    '#default_value' => $element['#value']['second'],
    '#options' => $element['#options']['second'],
    '#prefix' => ' and ',
    '#suffix' => '</div>',
    '#validated' => TRUE
  );

  $element['#validated'] = TRUE;

  return $element;
}

/**
 * Theme the custom form element
 */
function theme_pairwise_compare_germplasm ($element) {
  return ''; //theme('form_element', $element, '<div class="pairwise_compare_germplasm-element container-inline">'. $element['#children'] .'</div>');
}

/**
 * Validate user-input for all pairwise_compare_germplasm form elements
 */
function pairwise_compare_germplasm_validate ($element) { }

