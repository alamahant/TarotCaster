#include "meaningdisplay.h"

MeaningDisplay::MeaningDisplay(QWidget *parent) : QTextEdit(parent)

{
    setReadOnly(true);
    setStyleSheet("QTextEdit { background-color: black; color: gold; }");
}




void MeaningDisplay::displayMeaning(const CardMeaning& meaning) {

    QString html = "<div style='color: gold;'>";
    html += "<h2 style='color: gold;'>" + meaning.getName() + "</h2>";

    html += "<h3 style='color: gold;'>Keywords:</h3><ul>";
    for(const QString& keyword : meaning.getKeywords()) {
        html += "<li style='color: gold;'>" + keyword + "</li>";
    }
    html += "</ul>";

    html += "<h3 style='color: gold;'>Light Meanings:</h3><ul>";
    for(const QString& light : meaning.getLightMeanings()) {
        html += "<li style='color: gold;'>" + light + "</li>";
    }
    html += "</ul>";

    html += "<h3 style='color: gold;'>Shadow Meanings:</h3><ul>";
    for(const QString& shadow : meaning.getShadowMeanings()) {
        html += "<li style='color: gold;'>" + shadow + "</li>";
    }
    html += "</ul>";
    html += "</div>";

    setHtml(html);
}
