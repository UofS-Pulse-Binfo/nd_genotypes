/**
 * @file
 * Contains two Drupal Behaviours which use AJAX to display the number of records/variants
 * for the current genotype matrix search. This is done asynchronously because the queries
 * can be quite slow (the time it takes to count the number of records increases as the
 * number of results increases).
 *
 * Logic:
 *   1) Add element in template that you want shown if JS is unavailable: <span class="ajax-waiting">Unavailable</span>
 *   2) Use JS to replace element in #1 with "Counting" message and Drupal AJAX spinner.
 *   3) Use jQuery to retrieve the count via a JSON Callback. Specs of the genotype matrix
 *      are stored in the query parameters ($_GET) just as they are on the matrix page.
 *   4) When the count is returned, replace the "Counting" message with the actual count :-).
 * NOTE: On failure, the AJAX spinner will continue into infinity... :-(
 * @todo: look into feedback to the user in the future.
 *
 * @inspiration https://www.drupal.org/docs/7/api/javascript-api/ajax-in-drupal-using-jquery
 */
(function ($) {

  // 1) Number of results
  Drupal.behaviors.genotypeMatrixGetResultCounts = {
    attach: function (context, settings) {

      // First give the user some indication we are working --Also useful for admin ;-).
      $('.matrix-counts .result-count .ajax-waiting').replaceWith('<span class="ajax-progress ajax-progress-throbber">Counting<div class="throbber"> </div></span>');

      // Next, use jQuery to retrieve the count.
      var url = '/chado/genotype/' + Drupal.settings.NDgenotypes.partition + '/num_results.json' + location.search;
      console.log(url);
      $.getJSON(url,
        function(data) {
          // Once the count has been retrieved, substitute it into the page.
          $('.matrix-counts .result-count .ajax-progress').replaceWith('<span class="number">' + data.formatted + '</span>');
      });
    }
  };

  // 2) Number of variants
  // This is different from #1 since each variant might have multiple locations.
  Drupal.behaviors.genotypeMatrixGetVariantCounts = {
    attach: function (context, settings) {

      // First give the user some indication we are working --Also useful for admin ;-).
      $('.matrix-counts .variant-count .ajax-waiting').replaceWith('<span class="ajax-progress ajax-progress-throbber">Counting<div class="throbber"> </div></span>');

      // Next, use jQuery to retrieve the count.
      var url = '/chado/genotype/' + Drupal.settings.NDgenotypes.partition + '/num_variants.json' + location.search;
      $.getJSON(url,
        function(data) {
          // Once the count has been retrieved, substitute it into the page.
          $('.matrix-counts .variant-count .ajax-progress').replaceWith('<span class="number">' + data.formatted + '</span>');
      });
    }
  };

}(jQuery));
