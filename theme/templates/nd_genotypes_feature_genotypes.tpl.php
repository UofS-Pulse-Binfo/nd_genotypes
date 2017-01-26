<?php
/**
 *
 */

if (($type == 'marker' OR $type == 'variant') AND $no_genotypes === FALSE) :

$matrix_url = url(
  'chado/genotype/' . $node->nd_partition,
  array(
    'query' => array('variant_name' => array( $variant['variant_name'] )),
  )
);
$variant_url = url(
  'node/' . $variant['variant_nid'],
  array(
    'query' => array('pane' => 'genotypes'),
  )
);
?>

<style>
  #nd-genotypes-pie-chart {
    padding: 50px 20px;
  }
  .arc.not-the-focus {
    mix-blend-mode: screen;
  }
</style>


<div class="tripal_feature-data-block-desc tripal-data-block-desc">
  <?php if ($type == 'variant') { ?>
    The current variant has genotypes generated from <?php print sizeof($variant['marker_alleles']); ?> different
    marker(s). The distribution of alleles for each marker is shown below as
    one ring composing the pie chart. This allows you to compare the distribution across
    marker(s), as well as, get an overall idea of the distribution of alleles.
  <?php } else { ?>
    The distribution of alleles for the current marker is shown below as
    coloured portions of the pie chart where each colour represents an observed allele.
  <?php } ?>
</div>

<div id="nd-genotypes-pie-chart"></div>

<div>For germplasm-specific genotype calls: <a href="<?php print $matrix_url; ?>" target="_blank">Genotype Matrix</a>.</div>
<?php if ($type == 'marker') : ?>
  <div>For all marker types at this locus: <a href="<?php print $variant_url; ?>" target="_blank">Variant Page: Genotypes</a>.</div>
<?php endif; ?>

<?php endif; ?>
