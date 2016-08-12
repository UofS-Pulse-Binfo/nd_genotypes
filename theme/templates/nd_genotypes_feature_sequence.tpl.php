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
if (isset($sequence) AND !empty($sequence)) :
?>

<style>

  /* Make the sequence look like it's on the terminal ;-). */
  #tripal_feature-fasta-record {
        font-family: Consolas, monaco, monospace;
  }
  #tripal_feature-sequence-residues {
    width: 420px;
    word-wrap: break-word;
    margin: 0;
    letter-spacing:3px;
    line-height: 18px;
    padding-top: 0;
    margin-top: 0;
    background-color: transparent;
    text-align: justify;
    font-size: 12px;
  }

  /* Make sure the SNPs are noticeable */
  .variant-marked-up-sequence .variant {
    font-weight: bold;
    font-size: 14px;
    letter-spacing: 2px;
  }
  .variant-marked-up-sequence .variant-expanded a {
    color: blue;
  }
</style>

  <div class="tripal_feature-data-block-desc tripal-data-block-desc"></div>

    <!-- Variant Marked-up Sequence -->
    <?php if (!empty($sequence_with_variants)) { ?>
      <h3>Variant Marked-up Sequence</h3>
      <?php print '<p>' . $marked_description . '</p>'; ?>

      <div id="tripal_feature-fasta-record">
        <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
        <div id="tripal_feature-sequence-residues" class="variant-marked-up-sequence">
          <?php print $sequence_with_variants ?>
        </div>
      </div>
      <br />
    <?php } else { drupal_set_message(t('Unable to determine the Marked-up Sequence for this :type', array(':type' => $node->feature->type_id->name)), 'warning'); }?>

    <!-- Plain Sequence -->
    <h3>FASTA Record</h3>
    <?php print '<p>' . $fasta_description . '</p>'; ?>

    <div id="tripal_feature-fasta-record">
      <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
      <div id="tripal_feature-sequence-residues">
        <?php print $sequence; ?>
      </div>
    </div>

<?php endif; ?>
