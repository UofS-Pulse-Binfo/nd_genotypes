<?php
/**
 *
 */
$matrix_url = url(
  'chado/genotype/Lens',
  array(
    'query' => array('variant_name' => array('Chr1p801')),
  )
);
$variant_url = url(
  'feature/Lens/culinaris/SNP/Chr1p9425',
  array(
    'query' => array('pane' => 'genotypes'),
  )
);

if (($type == 'marker' OR $type == 'variant') AND $no_genotypes === FALSE) :
?>

<style>
  #nd-genotypes-pie-chart {
    padding: 50px 20px;
  }
  .arc.not-the-focus {
    mix-blend-mode: screen; //screen; //color-dodge;
  }
</style>


<div class="tripal_feature-data-block-desc tripal-data-block-desc">
  <?php if ($type == 'variant') { ?>
    The current variant has genotypes generated from <?php print sizeof($variant['marker_alleles']); ?> different
    marker technologies. The distribution of alleles for each tecnology is shown below as
    one ring composing the pie chart. This allows you to compare the distribution across
    marker technologies, as well as, get an overall idea of the distribution of alleles.
  <?php } else { ?>
    The distribution of alleles for the current marker technology is shown below as
    coloured portions of the pie chart where each colour represents an observed allele.
  <?php } ?>
</div>

<div id="nd-genotypes-pie-chart"></div>

<div>For specific genotype calls: <a href="<?php print $matrix_url; ?>" target="_blank">Genotype Matrix</a>.</div>
<div>For comparison with other markers: <a href="<?php print $variant_url; ?>" target="_blank">Variant Genotypes</a>.</div>

<?php endif; ?>
