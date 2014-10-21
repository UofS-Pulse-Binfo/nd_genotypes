<?php
/*
 * Details about genotypes associated with features can be found in the following way:
 *
 * feature => feature_genotype => genotype
 *
 * There are two ways that features with genotypes can be associated with stocks.  The first,
 * more simple method, is by traversion the FK relationships in this manner:
 *
 *   Simple Method: feature => feature_genotype => genotype => stock_genotype => stock
 *
 * The second method involves use of the natural diversity tables which allows for association
 * or more ancilliary information. Within the Natural Diversity tables, if a feature has genotypes then
 * you can find the corresponding stock by traversing the FK relationships
 * in this manner:
 *
 *   ND Method:     feature => feature_genotype => nd_experiment_genotype => nd_experiment => nd_experiment_stock => stock
 *
 * The tripal_natural_diversity module handles association of stocks using the ND method.
 * This template handles association of stocks when stored using the simple method.
 * If the tripal_natural_diversity module is enabled then this template will not show.
 * You should instead see the tripal_feature.nd_genotypes.tpl.php template
 *
 */
$feature = $variables['node']->feature;

// specify the number of genotypes to show by default and the unique pager ID
$num_results_per_page = 25;
$feature_pager_id = 15;

$is_variant = FALSE;
$is_marker = FALSE;

/////////////////////////////
// CASE #1: Markers

$marker_types = unserialize(variable_get('nd_genotypes_marker_types', 'a:0:{}'));
if (in_array($feature->type_id->name, $marker_types)) {

  $is_marker = TRUE;

  $options = array(
    'return_array' => 1,
  );
  $feature = tripal_core_expand_chado_vars($feature, 'table', 'feature_genotype', $options);
  $feature_genotypes = $feature->feature_genotype->feature_id;

  // get the total number of records
  $total_records = (sizeof($feature_genotypes) < $num_results_per_page) ? sizeof($feature_genotypes) : $num_results_per_page;

  // now iterate through the feature genotypes and print a paged table.
  if (count($feature_genotypes) > 0) {
    // the $headers array is an array of fields to use as the colum headers.
    // additional documentation can be found here
    // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
    $headers = array('Variant Type', 'Marker Call (Allele)');

    // the $rows array contains an array of rows where each row is an array
    // of values for each column of the table in that row.  Additional documentation
    // can be found here:
    // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
    $rows = array();

    foreach($feature_genotypes as $feature_genotype) {
      $genotype = $feature_genotype->genotype_id;


      // get the genotype type
      $type = 'N/A';
      if ($genotype->type_id) {
        $type = ucwords(preg_replace('/_/', ' ', $genotype->type_id->name));
      }

      // add the fields to the table row
      $rows[] = array(
        $type,
        $genotype->description,
      );
    }

  // Add in my pie chart :)
  $view = views_get_view('nd_genotypes_feature_genotype_chart_pie');
  if (!empty($view)) {
    $view->display['default']->display_options['filters']['feature_id']['value']['value'] = $feature->feature_id;
    print $view->preview('marker_block');
  }
  ?>

  <div class="tripal_feature-data-block-desc tripal-data-block-desc">The following <?php print number_format($total_records) ?> genotype(s) have been recorded for this feature.</div><?php

  // the $table array contains the headers and rows array as well as other
  // options for controlling the display of the table.  Additional
  // documentation can be found here:
  // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
  $table = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_genetic-table-genotypes',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );

  // once we have our table array structure defined, we call Drupal's theme_table()
  // function to generate the table.
  print theme_table($table);

  }
}


/////////////////////////////
// CASE #2: Variants

$variant_types = unserialize(variable_get('nd_genotypes_variant_types', 'a:0:{}'));
if (in_array($feature->type_id->name, $variant_types)) {

  $is_variant = TRUE;
  $variant_charts = array();

  $options = array( 'return_array' => 1 );
  $feature = tripal_core_expand_chado_vars($feature, 'table', 'feature_relationship', $options);

  if (isset($feature->all_relationships['object']['is marker of']['marker'])) {
    $markers = $feature->all_relationships['object']['is marker of']['marker'];
?>
  <div class="tripal_feature-data-block-desc tripal-data-block-desc">The following markers are associated with this variant and the genotypes for each marker are displayed below.</div>
  <table id="tripal_feature-variant_genotypes">
<?php
      // the $headers array is an array of fields to use as the colum headers.
      // additional documentation can be found here
      // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
      $headers = array('Variant Type', 'Marker Call (allele)');

      foreach ($markers as $marker) {

        $marker_feature_id = $marker->record->subject_id->feature_id;
        $marker_name = $marker->record->subject_id->name;
        if (isset($marker->record->subject_id->nid)) {
          $marker_name = l($marker_name, 'node/'.$marker->record->subject_id->nid);
        }

        $marker_genotypes = chado_generate_var('feature',array('feature_id' => $marker->record->subject_id->feature_id));
        $marker_genotypes = chado_expand_var($marker_genotypes, 'table', 'feature_genotype', array('return_array' => TRUE));

        $num_genotypes = sizeof($marker_genotypes->feature_genotype->feature_id);

        $rows = array();

        if (!empty($marker_genotypes->feature_genotype->feature_id)) {
          foreach ($marker_genotypes->feature_genotype->feature_id as $k => $genotype) {
            $rows[] = array(
              $genotype->genotype_id->type_id->name,
              $genotype->genotype_id->description
            );
          }
        }
?>
    <tr>
      <td class="layout genotypes-table">
      <?php
        // The table and marker description cell
        print '<h3>'.$marker_name.'</h3>';

        if (!empty($rows)) {
          // the $table array contains the headers and rows array as well as other
          // options for controlling the display of the table.  Additional
          // documentation can be found here:
          // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
          $table = array(
            'header' => $headers,
            'rows' => $rows,
            'attributes' => array(
              'id' => 'tripal_genetic-table-genotypes',
            ),
            'sticky' => FALSE,
            'caption' => '',
            'colgroups' => array(),
            'empty' => '',
          );

          // once we have our table array structure defined, we call Drupal's theme_table()
          // function to generate the table.
          print theme_table($table);
        }
        else {
          print '<p id="genotypes" class="no-alleles">No allele calls have been made available for this marker.</p>';
        }
      ?>
      </td>
      <td class="layout genotypes-pie">
      <?php
        // The Genotype Pie Chart cell
        $view = views_get_view('nd_genotypes_feature_genotype_chart_pie');
        if (!empty($view)) {
          $view->display['default']->display_options['filters']['feature_id']['value']['value'] = $marker_feature_id;
          print $view->preview('marker_block');
        }
      ?>
      </td>
    </tr>
<?php
      } //end of foreach marker
?>
  </table>
<?php
  } // end of if markers associated with variant
} //end of if variant
