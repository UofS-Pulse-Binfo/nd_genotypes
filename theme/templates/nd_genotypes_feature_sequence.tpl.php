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
<div id="sequence-tripal-data-pane-content">
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
    a:link.highlight-loc {
      font-weight: bolder;
      text-decoration: underline;
    }
  </style>

  <div class="tripal_feature-data-block-desc tripal-data-block-desc"></div>

    <!-- Locations -->
    <?php
      if (sizeof($locations) == 1) {
        print '<strong>Locations:</strong> ' . $locations[0]->backbone_name . ':' . $locations[0]->fmin . '-' . $locations[0]->fmax . '<br />';
      }
      else {
        print '<strong>Locations:</strong><ul>';
        $curr_locgroup = NULL;
        $q = drupal_get_query_parameters();
        foreach ($locations as $loc) {

          // If this is the first member of a locgroup then I should print out the last one.
          if ($curr_locgroup != $loc->locgroup) {
            if (!empty($cur_list)) {
              print '<li>'.implode(', ', $cur_list).'</li>';
            }
            $curr_locgroup = $loc->locgroup;
            $cur_list = array();
          }

          // Generate an ajax link for this location.
          $loc_name = $loc->backbone_name . ':' . $loc->fmin . '-' . $loc->fmax;
          $q['seq-loc'] = $loc;
          $link = array(
            '#type' => 'link',
            '#title' => t($loc_name),
            '#href' => '/node/'.$node->nid.'/ajax/sequences/'.$loc_name.'/nojs',
            '#ajax' => array(
              'effect' => 'fade',
            ),
          );

          // If this location is the currently displayed location then we want to
          // highlight it.
          if ($loc->featureloc_id == $current_location->featureloc_id) {
            $link['#options']['attributes']['class'][] = 'highlight-loc';
          }

          $cur_list[] = drupal_render($link);
        }
        if (!empty($cur_list)) {
          print '<li>'.implode(', ', $cur_list).'</li>';
        }
        print '</ul>';
      }?>
    <!-- Checkbox
    <input type="checkbox" name="marked_up" value="1" checked> Show variants in sequence<br>-->
    <br />
    <!-- Variant Marked-up Sequence -->
    <?php if (!empty($sequence_with_variants)) { ?>
      <strong>Variant Marked-up Sequence</strong>
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
    <strong>FASTA Record</strong>
    <?php print '<p>' . $fasta_description . '</p>'; ?>

    <div id="tripal_feature-fasta-record">
      <div id="tripal_feature-sequence-header"><?php print $fasta_header; ?></div>
      <div id="tripal_feature-sequence-residues">
        <?php print $sequence; ?>
      </div>
    </div>
</div>
<?php endif; ?>
