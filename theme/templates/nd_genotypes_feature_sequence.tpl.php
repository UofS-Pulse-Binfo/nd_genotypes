<?php
/*
 * There are two ways that sequences can be displayed.  They can come from the
 * feature.residues column or they can come from an alignment with another feature.
 * This template will show both or one or the other depending on the data available.
 *
 * For retreiving the sequence from an alignment we would typically make a call to
 * tripal_core_expand_chado_vars function.  For example, to retrieve all
 * of the featurelocs in order to get the sequences needed for this template, the
 * following function call would be made:
 *
 *   $feature = tripal_core_expand_chado_vars($feature,'table','featureloc');
 *
 * Then all of the sequences would need to be retreived from the alignments and
 * formatted for display below.  However, to simplify this template, this has already
 * been done by the tripal_feature module and the sequences are made available in
 * the variable:
 *
 *   $feature->featureloc_sequences
 *
 */

$feature = $variables['node']->feature;

// we don't want to get the sequence for traditionally large types. They are
// too big,  bog down the web browser, take longer to load and it's not
// reasonable to print them on a page.
$residues ='';
if(strcmp($feature->type_id->name,'scaffold') !=0 and
   strcmp($feature->type_id->name,'chromosome') !=0 and
   strcmp($feature->type_id->name,'supercontig') !=0 and
   strcmp($feature->type_id->name,'pseudomolecule') !=0) {
  $feature = tripal_core_expand_chado_vars($feature,'field','feature.residues');
  $residues = $feature->residues;
}

// get the sequence derived from alignments
$feature = $variables['node']->feature;

$type = 'other';
$info = array();
$info['sequence'] = NULL; // Needed for the check later
$variant_types = array('SNP', 'MNP','indel');
if ($feature->type_id->name == 'marker') {
  $type = 'marker';

  $feature_loc = chado_generate_var(
    'featureloc',
    array('feature_id' => $feature->feature_id),
    array('include_fk' => array('srcfeature_id' => array('type_id' => TRUE)))
  );
  $feature_loc = chado_expand_var($feature_loc,'field','feature.residues');
  $parent_feature = $feature_loc->srcfeature_id;
  $info['srcfeature_id'] = $parent_feature->feature_id;
  $info['feature_position'] = $feature_loc->fmin;

  $sequence_feature_name = $parent_feature->name;
  $fasta_description = 'The following sequence is that of the parent ' . $parent_feature->type_id->name
    . ' <strong>without any variants, including the current one, taken into account</strong>.';
  $marked_description = 'The following sequence is that of the parent '
    . $parent_feature->type_id->name . ' <strong>modified to highlight all of the known variants</strong>.
    The main variant is displayed using the [allele1/allele2] notation and any other
    variants are represented via their IUPAC code. The resulting FASTA is in the <strong>
    format required by most marker development programs including that for KASP assays</strong>.';

  $info['sequence'] = $parent_feature->residues;
}
elseif (in_array($feature->type_id->name, $variant_types)) {
  $type = 'variant';

  $feature_loc = chado_generate_var(
    'featureloc',
    array('feature_id' => $feature->feature_id),
    array('include_fk' => array('srcfeature_id' => array('type_id' => TRUE)))
  );
  $feature_loc = chado_expand_var($feature_loc,'field','feature.residues');
  $parent_feature = $feature_loc->srcfeature_id;
  $info['srcfeature_id'] = $parent_feature->feature_id;
  $info['feature_position'] = $feature_loc->fmin;

  $sequence_feature_name = $parent_feature->name;
  $fasta_description = 'The following sequence is that of the parent ' . $parent_feature->type_id->name
    . ' <strong>without any variants, including the current one, taken into account</strong>.';
  $marked_description = 'The following sequence is that of the parent '
    . $parent_feature->type_id->name . ' modified to highlight all of the known variants.
    The main variant is displayed using the [allele1/allele2] notation and any other
    variants are represented via their IUPAC code. The resulting FASTA is in the format
    required by most marker development programs including that for KASP assays.';

  $info['sequence'] = $parent_feature->residues;
}
elseif (!empty($residues)) {
  $type = 'sequence';

  $sequence_feature_name = $feature->name;
  $fasta_description = 'The following sequence is that of the ' . $feature->type_id->name
    . ' <strong>without any variants taken into account</strong>.';
  $marked_description = '';

  $info['sequence'] = $feature->residues;
  $info['feature_position'] = 0;
}
if ($info['sequence']) { ?>
  <div class="tripal_feature-data-block-desc tripal-data-block-desc"></div> <?php

  // Marked-up record
  $sequence_details = nd_genotypes_markup_variant_sequence($feature->feature_id, $type, $info);
  $markedup_sequence = $sequence_details['marked_up_sequence'];

  $fasta_header = '>' . $sequence_feature_name . ':' . $sequence_details['start'] . '-' . $sequence_details['end'] . ' (' . $feature->uniquename . ': ' . $feature->type_id->name . ')';

  $has_variants = TRUE;
  if ($markedup_sequence == $sequence_details['shortened_sequence']) {
    $has_variants = FALSE;
  }
  if ($markedup_sequence AND $has_variants) { ?>
    <h3>Variant Marked-up Sequence</h3>
    <?php print '<p>' . $marked_description . '</p>'; ?>

    <div id="tripal_feature-fasta-record">
    <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
    <pre id="tripal_feature-sequence-residues" class="variant-marked-up-sequence"><?php
      print $markedup_sequence; ?>
    </pre>
    </div> <?php
  }

  // FASTA Record
  ?>
    <br />
    <h3>FASTA Record</h3>
    <?php print '<p>' . $fasta_description . '</p>'; ?>

    <div id="tripal_feature-fasta-record">
    <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
    <pre id="tripal_feature-sequence-residues"><?php
      print $sequence_details['shortened_sequence']; ?>
    </pre>
    </div><?php
}