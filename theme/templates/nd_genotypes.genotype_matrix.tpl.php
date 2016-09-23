<?php
/**
 * @file
 * The Genotype Matrix (form + results).
 * @POST/Redirect/GET: The drupal cycle of POST/Redirect/GET means that this template function gets
 * called twice. First, when the form is submitted (POST) with the old values and then
 * again after redirect (GET) with the new values.
 */

$settings = array(
  'total_num_rows' => 100,
);

// Initialize the table and other pertinent variables.
$table = array(
  'header' => $header,
  'rows' => array(),
);
$missing_list['germplasm'] = (isset($query['args'][':germplasm'])) ? array_combine($query['args'][':germplasm'],$query['args'][':germplasm']) : array();
$missing_list['variants'] = (isset($defaults['variant_name'])) ? array_combine($defaults['variant_name'],$defaults['variant_name']) : array();

// Limit the query based on the number of germplasm.
$settings['query_limit'] = ($settings['total_num_rows'] + 1) * sizeof($germplasm);
$query['sql'][] = '  LIMIT ' . $settings['query_limit'];

// And execute the query :-).
// @see POST/Redirect/GET note above: Obviously, we only want to execute the heavy query
// on the new data and not the old post data. As such, only execute on GET (ie: page refresh).
$resource = array();
if ($_SERVER['REQUEST_METHOD'] =='GET' AND $settings['query_limit'] > 0 AND $query['args'][':germplasm']) {
  $resource = chado_query(implode(' ',$query['sql']), $query['args']);
}

// Generate the table rows.
// Keep in mind that we retrieve the data as a long listing rather than a table
// and join it ourself based on variant_id.
$last_loc = NULL;
$first_loc = NULL;
$next_page_loc = NULL;
$num_rows = 0;
$curr_row = NULL;
$l_options = array('attributes' => array( 'target' => '_blank' ));
foreach($resource as $r) {

  $row_id = $r->srcfeature_name . ':' .$r->fmin . '_' . $r->variant_id;

  // If this is the first record of the next row, finish off the last row first!
  if ($curr_row != $row_id AND $curr_row !== NULL) {

    // Determine the consensus call for each germplasm.
    foreach ($table['rows'][$curr_row] as $germplasm_id => $alleles) {
      if (isset($germplasm[ $germplasm_id ]) AND is_array($alleles)) {

        $consensus_allele = nd_genotype_get_consensus_call($alleles);
        $table['rows'][$curr_row][$germplasm_id] = array(
          'data' => $consensus_allele,
          'class' => array('germplasm', $germplasm[ $germplasm_id ]['class'], 'genotype', $consensus_allele)
        );
      }
    }
  }
  $curr_row = $row_id;

  // Save the first location for use with the pager.
  if (!isset($first_loc)) {
    $first_loc = $r->srcfeature_name . ':' .$r->fmin;
  }

  // Initialize the variant if this is the first time we have come across it.
  if (!isset($table['rows'][$row_id])) {
    // First initialize the row with empty values.
    $table['rows'][$row_id] = $template_row;

    // Then fill in the core information.
    if ($r->nid) {
      $table['rows'][$row_id]['variant_name'] = array(
        'data' => l($r->variant_name, 'node/'.$r->nid, $l_options),
        'class' => array('variant_name')
      );
    }
    else {
      $table['rows'][$row_id]['variant_name'] = array(
        'data' => $r->variant_name,
        'class' => array('variant_name')
      );
    }
    $table['rows'][$row_id]['srcfeature_name'] = array(
      'data' => $r->srcfeature_name,
      'class' => array('position','backbone')
    );
    $table['rows'][$row_id]['fmin'] = array(
      'data' => $r->fmin,
      'class' => array('position', 'start')
    );
    $table['rows'][$row_id]['fmax'] = array(
      'data' => $r->fmax,
      'class' => array('position','end')
    );
  }

  // Since we have data for this variant, remove it from the missing list.
  unset($missing_list['variants'][$r->variant_name]);
  // Since we have data for this germplasm, remove it from the missing list.
  unset($missing_list['germplasm'][$r->germplasm_id]);

  // Determine whether we are still within our row limit.
  $num_rows = sizeof($table['rows']);
  if ($num_rows > $settings['total_num_rows']) {
    $next_page_loc = $r->srcfeature_name . ':' . $r->fmin;
    unset($table['rows'][$row_id]);
    break;
  }

  // Save the last location for use with the pager.
  $last_loc = $r->srcfeature_name . ':' . $r->fmin;

  // Save the call. We will determine the consensus and mark it up at the end of the row.
  $table['rows'][$row_id][$r->germplasm_id][] = $r->allele;

  $num_rows++;
}

// Determine the consensus call for each germplasm in the last row.
if (isset($table['rows'][$curr_row])) {
  foreach ($table['rows'][$curr_row] as $germplasm_id => $alleles) {
    if (isset($germplasm[ $germplasm_id ]) AND is_array($alleles)) {
      $consensus_allele = nd_genotype_get_consensus_call($alleles);
      $table['rows'][$curr_row][$germplasm_id] = array(
        'data' => $consensus_allele,
        'class' => array('germplasm', $germplasm[ $germplasm_id ]['class'], 'genotype', $consensus_allele)
      );
    }
  }
}

// Use the missing lists to warn users but only if the number of results is less than
// the number of rows for a page.
// @see POST/Redirect/GET note above: We check here to keep the messages
// from being duplicated; not to mention, to keep incorrect, stale messages from
// confusing the user.
if ($_SERVER['REQUEST_METHOD'] =='GET' AND $query['args'][':germplasm'] AND sizeof($table['rows']) > 0) {
  if (sizeof($table['rows']) < $settings['total_num_rows']) {
    if (!empty($missing_list['germplasm'])) {
      foreach ($missing_list['germplasm'] as $germ_id => $v) {
        $missing_list['germplasm'][$germ_id] = $germplasm[$germ_id]['name'];
      }
      drupal_set_message('Because your results fit on a single page, we are able to determine that the following '.sizeof($missing_list['germplasm']).' germplasm do not have any allele calls under the specified fitler criteria: '.implode(', ', $missing_list['germplasm']).'.', 'warning');
    }
    if (!empty($missing_list['variants'])) {
      drupal_set_message('Because your results fit on a single page, we are able to determine that the following '.sizeof($missing_list['variants']).' variants do not have any allele calls under the specified filter criteria: '.implode(', ', $missing_list['variants']).'.','warning');
    }
  }

  // Now determine the URL for the pager.
  $pager = nd_genotypes_genotype_matrix_get_pager(array(
    'first_loc' => $first_loc,
    'last_loc' => $last_loc,
    'next_page_loc' => $next_page_loc,
    'max_rows' => $settings['total_num_rows'],
    'num_rows' => $num_rows,
    'query' => $query,
  ));
}

// Since the germplasm list is absolutely critical to getting results, (if they have
// parameters but haven't set that (only possibly through links or modifying the URL)
// we should warn them if it's not set.
if ($_SERVER['REQUEST_METHOD'] =='GET' AND !$query['args'][':germplasm']) {
  drupal_set_message('Enter the names of the germplasm you would like to see the genotypes of under "Germplasm" below.', 'warning');
}
?>

<div id="genotype-matrix-<?php print $genus;?>">
  <div class="filter-form">
    <?php
      $form = drupal_get_form('nd_genotypes_matrix_filter_form', array('genus' => $genus, 'defaults' => $defaults));
      print drupal_render($form);
    ?>
  </div>

  <?php if ($settings['query_limit'] == 0) { ?>

    <div class="help-text">
      Please specify the germplasm you would like to see the genotypes of under "Germplasm" above.
    </div>

  <?php } elseif (empty($table['rows'])) { ?>

    <div class="no-results">
      No variants with genotypes matched the given filter criteria.
    </div>

  <?php } else { ?>

    <!-- <div class="matrix-download">Download:
      <?php print l('CSV', 'chado/genotype/'.$genus.'/csv', array('query' => drupal_get_query_parameters(), 'attributes' => array('target' => '_blank'))); ?>
    </div> -->
    <div class="matrix-proper">
      <?php print theme('table', $table); ?>
    </div>

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
