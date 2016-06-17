<?php
/**
 * @file
 * Template used to populate the help page for this module.
 */
?>

<h2>Module Description</h2>
<p>This module provides a more specialized interface to genotypic information
stored in the chado natural diversity module tables including adding summaries
of this information to feature pages and providing a powerful marker by germplasm
matrix (See features below).</p>

<br/>
<h2>Setup Instructions</h2>
<ol>
  <li><strong>Run the Tripal Jobs</strong> scheduled by this module when it was installed. These
  jobs include creation of the materialized views used to provide a unified
  data source for this module and, depending on the amount of genotypic data
  you've already loaded, may take awhile. This can be done by running
  <code>drush trp-run-jobs --user [administrator]</code> from the DRUPAL_ROOT of your
  site where [administrator] is the username of your administrative account.</li>
  <li><strong>Load your data</strong> using the Tripal Bulk Loader template
  provided by this module. To do this, <?php print l('create a Bulk Loading Job', ''); ?>
  with the template being "ND Genotype: Generic Genotype Experiments". Once you create the job, you
  will need to describe your data file. Each "Constant Set" refers to
  a single column in your data file and you will need to indicate the uniquename
  of the stock those genotypes belong to and the column number (both in
  "Allele Call" and "Unique Name (Generated)" where the first column is "1").
  Once you have enetered all this data (all fields are required) then you
  can submit the job for loading and run it on the command-line like you
  would any other tripal job (using drush trp-run-jobs).</li>
  <li><strong>Update the materialized views</strong> by going to Administration >
  Tripal > Extension Modules > Natural Diversity Genotypes and clicking on the
  <?php print l('Update tab', 'admin/tripal/extension/nd_genotypes/sync');?>.
  Since the nd_genotype_experiment materialized view has an extra step of
  updating polymorphic information, <em>this should <strong>not</strong> be done
  through the Tripal Materialized Views administrative interface</em>.</li>
  <li><strong>OPTIONAL: Install
  <a href="https://www.drupal.org/project/Highcharts">HighCharts module</a></strong>
   which requires the <a href="https://www.drupal.org/project/Libraries">Libraries</a>
   and <a href="https://www.drupal.org/project/Charts">Charts</a> Drupal modules,
   as well as, the <a href="http://www.highcharts.com/">Highcharts javascript
   library</a> which should be located at DRUPAL_ROOT/sites/all/libraries/highcharts.
   This module supports the pie charts described in the "Features" below.</li>
</ol>
<br />

<h2>Features of this Module</h2>
<ul>
  <li>Adds specialized panes to marker and variant features to better
  display/summarize genotypic data. Specifically, a "Genotypes" pane with genotype
  distribution pie chart (HighCharts module needed) and a "Sequences" pane which
  displays a region of the parent sequence for a marker with any known variants
  highlighted by their IUPAC wobble code to aid new marker design.</li>
  <li>Provides a search view displaying all the genotypes for a given stock in
  a marker (rows) by stock (columns) table. The user can indicate which stocks/germplasm
  they want to see and there is no set number of germplasm they can add.</li>
</ul>