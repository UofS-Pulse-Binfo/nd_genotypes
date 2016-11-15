<?php
/**
 * @file
 * The Genotype Matrix (form + results).
 * @POST/Redirect/GET: The drupal cycle of POST/Redirect/GET means that this template function gets
 * called twice. First, when the form is submitted (POST) with the old values and then
 * again after redirect (GET) with the new values.
 */

// Initialize the table and other pertinent variables.
$table = array(
  'header' => $header,
  'rows' => array(),
);
$missing_list['germplasm'] = (isset($query_args['germplasm_id'])) ? array_combine($query_args['germplasm_id'],$query_args['germplasm_id']) : array();
$missing_list['variants'] = (isset($defaults['variant_name'])) ? array_combine($defaults['variant_name'],$defaults['variant_name']) : array();

// Generate the table rows.
$l_options = array('attributes' => array( 'target' => '_blank' ));
$num_rows = 0;
$first_row = $last_row = array();
if (isset($variants)) {
  foreach($variants as $row_id => $v) {

    // First initialize the row with empty values.
    $table['rows'][$row_id] = $template_row;

    // Then fill in the core information.
    if ($v->nid) {
      $table['rows'][$row_id]['variant_name'] = array(
        'data' => l($v->variant_name, 'node/'.$v->nid, $l_options),
        'class' => array('variant_name')
      );
    }
    else {
      $table['rows'][$row_id]['variant_name'] = array(
        'data' => $v->variant_name,
        'class' => array('variant_name')
      );
    }
    $table['rows'][$row_id]['srcfeature_name'] = array(
      'data' => $v->srcfeature_name,
      'class' => array('position','backbone')
    );
    $table['rows'][$row_id]['fmin'] = array(
      'data' => $v->fmin,
      'class' => array('position', 'start')
    );
    $table['rows'][$row_id]['fmax'] = array(
      'data' => $v->fmax,
      'class' => array('position','end')
    );

    if (isset($data[$v->variant_id])) {
      foreach ($data[$v->variant_id] as $germplasm_id => $alleles) {
        if (isset($germplasm[ $germplasm_id ]) AND is_array($alleles)) {

          $consensus_allele = nd_genotype_get_consensus_call($alleles);
          $table['rows'][$row_id][$germplasm_id] = array(
            'data' => $consensus_allele,
            'class' => array('germplasm', $germplasm[ $germplasm_id ]['class'], 'genotype', $consensus_allele)
          );
        }
      }
    }

    // If this is the first row then save the location for use in the pager.
    if ($num_rows == 0) {
      $first_row = array(
        'srcfeature' => $v->srcfeature_name,
        'start' => $v->fmin,
      );
    }

    $num_rows++;
  }
}

// Now that we're don, if $r is set then thats the last row and we should save
// that location for the pager as well.
if (isset($v)) {
  $last_row = array(
    'srcfeature' => $v->srcfeature_name,
    'end' => $v->fmax,
  );
}

// And since the germplasm are absolutely critical we should warn them if it's not set.
if ($_SERVER['REQUEST_METHOD'] =='GET' AND !$query_args['germplasm_id']) {
  drupal_set_message('Enter the names of the germplasm you would like to see the genotypes of under "Germplasm" below.', 'warning');
}

// Determine the pager.
$settings['total_num_rows'] = 100;
$pager = array('label' => '');
$curr_path = current_path();
$query_param = drupal_get_query_parameters();
if (!isset($_GET['page'])) $_GET['page'] = 1;

if ($first_row AND $last_row) {
  if ($first_row['srcfeature'] == $last_row['srcfeature']) {
    $pager['label'] = strtr(
      '@sbackbone: @min-@max',
      array(
        '@sbackbone' => $first_row['srcfeature'],
        '@min' => $first_row['start'],
        '@max' => $last_row['end'],
      )
    );
  }
  else {
    $pager['label'] = strtr(
      '@sbackbone:@min - @ebackbone:@max',
      array(
        '@sbackbone' => $first_row['srcfeature'],
        '@min' => $first_row['start'],
        '@ebackbone' => $last_row['srcfeature'],
        '@max' => $last_row['end'],
      )
    );
  }
}
else {
  $pager['label'] = 'Page ' . $_GET['page'];
}

// Not the first page...
if ($_GET['page'] != 1) {

  // First page.
  $q = $query_param;
  $q['page'] = 1;
  $pager['first'] = url($curr_path, array('query' => $q));

  // Previous page.
  $q = $query_param;
  $q['page'] = $_GET['page'] - 1;
  $pager['prev'] = url($curr_path, array('query' => $q));
}

// Not the "last" page...
if ($_GET['page'] != 'last') {

  // Next page.
  $q = $query_param;
  $q['page'] = $_GET['page'] + 1;
  $pager['next'] = url($curr_path, array('query' => $q));

  // Last page.
  $q = $query_param;
  $q['page'] = 'last';
  $pager['last'] = url($curr_path, array('query' => $q));
}

// Also check for the last page if they actually paged through them all...
// Specifically, if the total number of records is between the beginning and end of the current page.
/*if (($_GET['page']*100-100 <= $total) AND ($_GET['page']*100 >= $total) ) {
  unset($pager['next'], $pager['last']);
}*/
if ($num_rows < 100) {
  unset($pager['next'], $pager['last']);
}

?>

<div id="genotype-matrix-<?php print $genus;?>">
  <div class="filter-form">
    <?php
      $form = drupal_get_form('nd_genotypes_matrix_filter_form', array('genus' => $genus, 'partition' => $partition, 'defaults' => $defaults));
      print drupal_render($form);
    ?>
  </div>

  <?php if (empty($query_args['germplasm_id'])) { ?>

    <div class="help-text">
      Please specify the germplasm you would like to see the genotypes of under "Germplasm" above.
    </div>

  <?php } elseif (empty($table['rows'])) { ?>

    <div class="no-results">
      No variants with genotypes matched the given filter criteria.
    </div>

  <?php } else { ?>


    <!--<div class="matrix-download">Download:
      <?php print l('CSV', 'chado/genotype/'.$genus.'/csv', array('query' => drupal_get_query_parameters(), 'attributes' => array('target' => '_blank'))); ?>
    </div>-->
    <div class="matrix-proper">
      <?php print theme('table', $table); ?>
    </div>

    <!-- Add a pager here -->
    <div class="pager item-list">
      <ul class="pager">
        <li class="pager-first">
          <?php if (isset($pager['first'])) { ?>
            <a href="<?php print $pager['first']?>">‹‹ first <?php print $settings['total_num_rows']?></a>
          <?php } else { ?>
            ‹‹ first <?php print $settings['total_num_rows']?>
          <?php } ?>
        </li>
        <li class="pager-previous">
          <?php if (isset($pager['prev'])) { ?>
            <a href="<?php print $pager['prev']?>">‹ previous</a>
          <?php } else { ?>
            ‹‹ previous
          <?php } ?>
        </li>
        <li class="pager-current"><?php print $pager['label']?></li>
        <li class="pager-next">
          <?php if (isset($pager['next'])) { ?>
            <a href="<?php print $pager['next']?>">next ›</a>
          <?php } else { ?>
            next ›
          <?php } ?>
        </li>
        <li class="pager-last">
          <?php if (isset($pager['last'])) { ?>
            <a href="<?php print $pager['last']?>">last <?php print $settings['total_num_rows']?> ››</a>
          <?php } else { ?>
            last <?php print $settings['total_num_rows']?> ››
          <?php } ?>
        </li>
      </ul>
    </div>

  <?php } ?>
</div>
