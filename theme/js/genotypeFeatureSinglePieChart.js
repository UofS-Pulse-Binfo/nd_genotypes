/**
 *
 */
Drupal.behaviors.ndGenotypesFeaturePieChart = {
  attach: function (context, settings) {

    data = Drupal.settings.nd_genotypes.genotypes;

    var width = 600,
        height = 250,
        maxRadius = height / 2,
        donutWidth = Math.floor(maxRadius/2),
        timbitRadius = maxRadius-donutWidth-4, //Inside joke, donutHole ;-).
        labelPadding = width - height - 300;

    // Retrieve a set of colours to use.
    var color = d3.scale.ordinal()
        .range(["#A3AF51", "#35424C", "#5D6F06", "#6b486b", "#a05d56", "#d0743c", "#ff8c00"]);

    // Remove previous canvas if existing.
    d3.select("#nd-genotypes-pie-chart").selectAll("svg").remove();

    // Create the SNG canvas.
    var svg = d3.select("#nd-genotypes-pie-chart").append("svg")
        .attr("width", width)
        .attr("height", height)
        .append("g")
        .attr("transform", "translate(" + height / 2 + "," + height / 2 + ")");

    // Draw the circle background.
    svg.append("circle")
      .attr("cx", 0)
      .attr("cy", 0)
      .attr("r", maxRadius-1)
      .attr("fill", "#808080");
    svg.append("circle")
      .attr("cx", 0)
      .attr("cy", 0)
      .attr("r", timbitRadius)
      .attr("fill", "#FFF");

    // Actually draw each ring/donut in the pie chart based on the data provided.
    drawPie(data[0], 0, maxRadius-2);

    // Label the alleles.
    drawAlleleLegend(-100);

    // Move the legend within the circle if it will fit.
    var legendHeight = jQuery('g.legend')[0].getBBox().height,
      legendWidth = jQuery('g.legend')[0].getBBox().width;
    if (((legendHeight + 10) > (timbitRadius*2)) || ((legendWidth + 10) > (timbitRadius*2))) {
      var legendOffset = 0 - (legendHeight / 2);
      svg.selectAll('g.legend')
        .attr("transform", "translate(" + (maxRadius + 50) + "," + legendOffset + ")");
    } else {
      var legendOffset = 0 - (legendHeight / 2);
      svg.selectAll('g.legend')
        .attr("transform", "translate(" + (0 - (legendWidth / 2)) + "," + legendOffset + ")");
    }

    /**
     * This function draws the segments of the pie chart. Each time this function
     * is called it draws one of the rings/donuts. The data provided should be
     * specific to that particular ring/donut, the index is the 0-indexed order
     * and the radius is the maximum radius for this ring.
     */
    function drawPie(_data, index, radius) {

      var arc = d3.svg.arc()
        .outerRadius(radius - 2)
        .innerRadius(radius - donutWidth);

      var pie = d3.layout.pie()
        .sort(null)
        .value(function(d) { return d.num; });

      var seriesName = "m" + _data.label.replace(/ /g,'-').replace(/[!\"#$%&'\(\)\*\+,\.\/:;<=>\?\@\[\\\]\^`\{\|\}~]/g, '');
      var ring = svg.append("g")
        .attr("class", "ring series " + seriesName);

      var g = ring.selectAll(".arc")
          .data(pie(_data.portions))
        .enter().append("g")
          .attr("class", function(d) { return "arc " + d.data.class + " " + d.data.label; });

      g.append("path")
          .attr("d", arc)
          .style("fill", function(d) { return color(d.data.label); })
          .append("svg:title")
            .text(function(d) { return d.data.label + ' (' + d.data.num + ' calls)'; });

    }

    /**
     * This function draws the series labels for each donut/ring in the
     * pie chart. The labels are aligned on the right side of the chart
     * and then lines connect the label to the outer boundry of each
     * ring.
     */
    function drawDonutLabel(_data, index, radius, outerRadius) {

      var seriesName = "m" + _data.label.replace(/ /g,'-').replace(/[!\"#$%&'\(\)\*\+,\.\/:;<=>\?\@\[\\\]\^`\{\|\}~]/g, '');

      var label = svg.selectAll('.series-labels')
        .append("g")
          .attr("class", "label")
          .attr("class", seriesName);

      // Print out the labels in reverse order
      // with each label lining up with the top of a donut.
      label.append('text')
        .attr('x', maxRadius + labelPadding)
        .attr('y', donutWidth * (index - data.length) + 10)
        .text(function(d) { return _data.label; });

      // Function to draw lines.
      var drawLine = d3.svg.line()
        .x(function(d) { return d.x; })
        .y(function(d) { return d.y; })
        .interpolate("linear");

      // Find the x coordinate on a circle when given y and the radius.
      function getPointX(y, radius) {
        return Math.cos(Math.asin(y/radius)) * radius;
      }

      var labelHeight = (radius - 2) * -1 + 5,
        xOnOuter = getPointX(labelHeight,outerRadius),
        offset = (donutWidth/2) * (data.length - index + 1);

      midRadius = radius - (donutWidth/2);
      labelHeight = donutWidth * (index - data.length) + 5;
      endY = donutWidth - 5;
      xOnOuter = getPointX(labelHeight,outerRadius)
      lineData = [  // A point just before (2px) the label.
                   { "x": maxRadius + labelPadding - 2,   "y": labelHeight},
                    // A point offset 10px from the edge of the outer arc
                    // where the line would eventually intersect the arc.
                   { "x": xOnOuter + 10,  "y": labelHeight},
                    // Draw line to donut.
                   { "x": getPointX(-endY, radius),  "y": -endY}
                 ];
      label.append('path')
        .attr("d", drawLine(lineData))
        .attr("stroke", "#808080")
        .attr("stroke-width", 2)
        .attr("fill", "none");
    }

    /**
     * This function draws the legend detailing which colour was used for each allele.
     * The legend drawn will simply be a coloured rectangle and label per allele.
     */
    function drawAlleleLegend(offset) {

      var legendRectSize = 18,
        legendSpacing = 4;
      var legendBox = svg.append("g")
        .attr('class', 'legend');

      var legend = legendBox.selectAll('.data-points')
        .data(color.domain())
        .enter()
        .append('g')
        .attr('class', 'data-points');

      legend.append('rect')
        .attr('width', legendRectSize)
        .attr('height', legendRectSize)
        .attr('x', 0)
        .attr('y', function(d,i) { return (legendRectSize + 4) * i;})
        .style('fill', color)
        .style('stroke', color);
      legend.append('text')
        .attr('x', legendRectSize + 4)
        .attr('y', function(d,i) { return ((legendRectSize+2) * i) + legendRectSize;})
        .text(function(d) { return d; });
    }
  }
};
