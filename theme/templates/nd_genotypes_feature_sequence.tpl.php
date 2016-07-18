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
?>

<style>
  #tripal_feature-sequence-residues {
    width: 400px;
    word-wrap: break-word;
    margin: 0;
    letter-spacing:2px;
    padding-top: 0;
    margin-top: 0;
    background-color: transparent;
  }

  .variant-marked-up-sequence .variant {
    font-weight: bold;
    font-size: 1.25em;
  }
  .variant-marked-up-sequence .variant-expanded a {
    color: blue;
  }
</style>

<?php
if ($sequence) : ?>
  <div class="tripal_feature-data-block-desc tripal-data-block-desc"></div>

    <!-- Variant Marked-up Sequence -->
    <?php if (!empty($sequence_with_variants)) { ?>
      <h3>Variant Marked-up Sequence</h3>
      <?php print '<p>' . $marked_description . '</p>'; ?>

      <div id="tripal_feature-fasta-record">
        <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
        <pre id="tripal_feature-sequence-residues" class="variant-marked-up-sequence">
          <?php print $sequence_with_variants ?>
        </pre>
      </div>
      <br />
    <?php } else { drupal_set_message(t('Unable to determine the Marked-up Sequence for this :type', array(':type' => $node->feature->type_id->name)), 'warning'); }?>

    <!-- Plain Sequence -->
    <h3>FASTA Record</h3>
    <?php print '<p>' . $fasta_description . '</p>'; ?>

    <div id="tripal_feature-fasta-record">
      <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
      <pre id="tripal_feature-sequence-residues">
        <?php print $sequence; ?>
      </pre>
    </div>

<?php endif; ?>
