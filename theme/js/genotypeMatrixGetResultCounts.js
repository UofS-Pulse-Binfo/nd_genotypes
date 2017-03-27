
(function ($) {

  Drupal.behaviors.genotypeMatrixGetResultCounts = {
    attach: function (context, settings) {

      $.ajax({
        url: 'chado/genotype/Lens/ajax_count',
        method: "GET",
        // This is simply grabbing $_GET... I hope...
        data: location.search.substr(1).split("&").reduce((o,i)=>(u=decodeURIComponent,[k,v]=i.split("="),o[u(k)]=v&&u(v),o),{}),
        dataType: "json",          // Type of the content we're expecting in the response
        success: function(data) {
          console.log(data);
          $('.matrix-counts .result-count .number').innerHTML=data.display;  // Place AJAX content inside the ajax wrapper div
        }

      });
    }
  };
}(jQuery));
