<?php

/**
 * This filter allows users to filter multiple "Genotype by Stock" fields at once.
 *
 * To set up a view to use this filter, add as many "Genotype by Stock" fields
 * as you want to allow your user to set. If you set these fields to "Exclude
 * from Display" then the filter will expose them once the user has set a value
 * for them. This will give the appearance of allowing your users to dynamically
 * add columns to their matrix while still giving you control over the max
 * number of columns they can access at once.
 *
 * NOTE: you need at least 1 "Genotype by Stock" field for this filter to work
 * since it will not add any content to your view alo
 */
class views_handler_filter_add_germplasm_genotype_by_stock_field_filter
  extends views_handler_filter_germplasm_genotype_by_stock {

  /**
   * Preprocess the filter values before the query is run so that the various
   * "Genotype by Germplasm" fields have access to the processed information
   * in order to form their query.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);

    // Determine hte number of fields we have to work with so that we can
    // give the user good warnings if they try to add a stock that there is no
    // corresponding field for.
    if (!isset($this->num_germplasm_genotype_fields)) {
      $this->num_germplasm_genotype_fields = 0;
      $this->max_germplasm_genotype_index = NULL;
      if (isset($this->view->field)) {
        foreach ($this->view->field as $k => $field) {
        if (preg_match('/^germplasm_genotype_*(\d*)/',$k, $match)) {
          $this->num_germplasm_genotype_fields++;
          $index = ($match[1]) ? $match[1] : 0;
          if ($this->max_germplasm_genotype_index < $index) {
            $this->max_germplasm_genotype_index = $index;
          }
        }
      }
      }
    }

    // We work with $GET directly to ensure that the values are saved between
    // $view objects but at the end we assign it to $this->values to ensure the
    // two stay in sync.
    if (isset($_GET['add_germplasm_genotype']['add_stock'])) {
      if (!empty($_GET['add_germplasm_genotype']['add_stock']['stock_name']) AND !empty($_GET['add_germplasm_genotype']['add_stock']['organism_id'])) {

        if (!isset($_GET['add_germplasm_genotype']['existing'])) {
          $_GET['add_germplasm_genotype']['existing'] = array();
        }

        // Check that we have a corresponding "Genotypes by Germplasm" field
        // to add germplasm to and if not, warn the user rather than carrying on.
        $num_stocks_in_filter = sizeof($_GET['add_germplasm_genotype']['existing']);
        if (($num_stocks_in_filter + 1) > $this->num_germplasm_genotype_fields) {
          drupal_set_message(
            t('You have reached the maximum number of stocks (:max) you can display genotypes for. As such, the stock you entered has not been added to the existing set.',
              array(':max' => $this->num_germplasm_genotype_fields)),
            'warning'
          );
        }
        else {
          // Try to get the stock indicated by the user.
          $add_stock = $_GET['add_germplasm_genotype']['add_stock'];
          $stock = tripal_get_stock(array(
            'name' => $add_stock['stock_name'],
            'organism_id' => $add_stock['organism_id']
          ));

          // If successfully able to find the stock then add it to the
          // existing germplasm but only if it has genotypes attached.
          if (is_object($stock)) {
            if (tripal_germplasm_has_nd_genotypes($stock->stock_id)) {
              $values = $_GET['add_germplasm_genotype'];
              $values['existing'][] = array(
                'stock_name' => $add_stock['stock_name'],
                'organism_id' => $add_stock['organism_id'],
                'stock_id' => $stock->stock_id,
              );
              unset($values['add_stock']);

              $_GET['add_germplasm_genotype'] = $values;
            }
            else {
              $msg = 'That stock does not have any genotypes associated with it. Keep in mind that
                you currently have to enter the full name of the DNA stock that was assayed. For example,
                "964a-46 LAM Plate1 Extraction" instead of just "964a-46". For a list of stocks assayed
                click on the "Germplasm Genotyped" Link in the menu under the genotype search.';
              drupal_set_message($msg,'error');
              tripal_report_error(
                'nd_genotype',
                TRIPAL_WARNING,
                $msg
              );
            }
          }
        }
      }
      // If they entered a stock name but no organism then iform them this is So Not Okay ;)
      elseif (!empty($_GET['add_germplasm_genotype']['add_stock']['stock_name'])) {
        drupal_set_message('You must enter the source species when adding genotypes.', 'error');
      }
      // If they didn't supply either stock name or organism_id then maybe they just want to remove things
      else {
        // remove empty add_stock
        unset($_GET['add_germplasm_genotype']['add_stock']);

        // Then check to see if any need to be removed
        if (isset($_GET['add_germplasm_genotype']['existing'])) {
          foreach($_GET['add_germplasm_genotype']['existing'] as $k => $spec) {
            if (isset($spec['remove'])) {
              if ($spec['remove']) {
                unset($_GET['add_germplasm_genotype']['existing'][$k]);
              }
            }
          }
        }
      }

      // Make sure to assign the $GET values to $this to ensure it's
      // where the fields expect it.
      $this->value = $_GET['add_germplasm_genotype'];

    } //end of if add_germplasm_genotype is set

  }

  /**
   * Modify the options form.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['germplasm_expected'] = array(
      '#type' => 'checkbox',
      '#title' => 'Germplasm Expected?',
      '#description' => 'Warn the user if the Submit button is clicked but no germplasm entered.',
      '#default_value' => (isset($this->options['germplasm_expected'])) ? $this->options['germplasm_expected'] : TRUE
    );

    unset($form['operator'], $form['case']);
  }

  /**
   * Set-up the exposed form for this filter.
   */
  function value_form (&$form, &$form_state) {

    $form_element_name = $this->options['id'];

    // GET OPTIONS BASED ON ORGANISM ARGUEMENTS.
    if (!empty($this->view->argument)) {
      $org_values = array();
      foreach ($this->view->argument as $arg) {
        if ($arg->table == 'organism') {
          if (isset($this->view->args[ $arg->position ])) {
            $org_values[$arg->field] = $this->view->args[ $arg->position ];
          }
        }
      }
      if (!empty($org_values)) {
        $options = array('order_by' => array(
          'organism_id' => 'ASC'
        ));
        $organism = chado_select_record('organism',array('organism_id', 'genus', 'species'),$org_values,$options);
        if (is_array($organism)) {
          $organism_options[0] = ' ';
          foreach ($organism as $o) {
            $organism_genus = $o->genus;
            $organism_options[$o->organism_id] = $o->genus .' '. $o->species;
          }
        }
      }
    }

    $form['value'] = array(
      '#type' => 'multiple_organism_stock',
      '#organism_title' => 'Source Species',
      '#default_value' => $this->value,
      '#organism_weight' => 10,
      '#added_description' => 'The following stocks have already had their genotypes added to the listing below:',
    );
    if (isset($organism_options) AND sizeof($organism_options) > 1) {
      $form['value']['#organism_options'] = $organism_options;
    }
    if (isset($organism_genus)) {
      $form['value']['#autocomplete_path'] = 'tripal_ajax/nd_genotypes/genotyped_germplasm/name_to_id/' . $organism_genus . '/only';
    }

  }

  /**
   * Allows the filter to modify the query.
   *
   * This is needed to ensure query is not inherited parent filter adds
   * feature_id IS NULL when value is empty
   */
  function query () {

    // We want to warn the user to enter stocks in order to see germplasm.
    $this->options['germplasm_expected'] = (isset($this->options['germplasm_expected'])) ? $this->options['germplasm_expected'] : TRUE;
    if ($this->options['germplasm_expected']) {
      if (isset($this->view->exposed_input['add_germplasm_genotype']) AND !isset($this->view->exposed_input['add_germplasm_genotype']['existing'])) {
        drupal_set_message('In order to display genotypes you need to indicate the stock under "Genotypes: Stock Name" and "Genotypes: Source Organism"', 'warning');
      }
    }
  }
}
