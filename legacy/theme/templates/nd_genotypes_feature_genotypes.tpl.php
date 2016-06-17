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

if ($num_alleles) {

  // the $headers array is an array of fields to use as the colum headers.
  // additional documentation can be found here
  // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
  $headers = array('Marker Call (Allele)', 'Number of Germplasm');

  // the $rows array contains an array of rows where each row is an array
  // of values for each column of the table in that row.  Additional documentation
  // can be found here:
  // https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
  $rows = array();

  foreach ($markers as $marker_id => $marker) {
    $marker_feature_id = $marker->feature_id;

    // Name of the marker
    $marker_name[$marker_feature_id] = $marker->name;
    if (isset($marker->nid)) {
      $marker_name[$marker_feature_id] = l($marker_name[$marker_feature_id], 'node/'.$marker->nid);
    }

    // Add in my pie chart :)
    $view_output[$marker_feature_id] = '';
    $view = views_get_view('nd_genotypes_feature_genotype_chart_pie');
    if (!empty($view)) {
      $view->display['default']->display_options['filters']['feature_id']['value']['value'] = $marker->feature_id;
      $view_output[$marker_feature_id] = $view->preview('marker_block');
    }

    // Add in allele rows for the table.
    $rows = array();
    foreach($alleles[$marker_feature_id] as $allele) {
      $rows[] = array(
        $allele->allele,
        $allele->num_germplasm
      );
    }

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
    $table_output[$marker_feature_id] = theme_table($table);
  }
}
?>

<?php if ($is_marker) : ?>

  <?php print $view_output[$marker->feature_id]; ?>
  <div class="tripal_feature-data-block-desc tripal-data-block-desc">
    The following <?php print number_format($num_alleles[$marker->feature_id]) ?> genotype(s) have been recorded for this feature.
  </div>
  <?php print $table_output[$marker->feature_id]; ?>

<?php elseif ($is_variant) : ?>

  <div class="tripal_feature-data-block-desc tripal-data-block-desc">
    The following markers are associated with this variant and the genotypes for each marker are displayed below.
  </div>
  <table id="tripal_feature-variant_genotypes">
    <?php foreach ($markers as $marker_id => $marker): ?>
      <tr>
        <td class="layout genotypes-table">
          <h3><?php print $marker_name[$marker_id] ?></h3>
          <?php print $table_output[$marker_id]; ?>
        </td>
        <td class="layout genotypes-pie">
          <?php print $view_output[$marker_id]; ?>
        </td>
      </tr>
    <?php endforeach; ?>
  </table>

<?php endif; ?>
