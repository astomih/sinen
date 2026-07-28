#include <core/license_report.hpp>

#include "licenses_data.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>

namespace sinen {
namespace {
int hexDigit(char value) {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return value - 'a' + 10;
  }
  if (value >= 'A' && value <= 'F') {
    return value - 'A' + 10;
  }
  return -1;
}

std::string decodeHex(const std::array<std::string_view, 4> &hexChunks) {
  std::string result;
  size_t encodedSize = 0;
  for (const StringView chunk : hexChunks) {
    if (chunk.size() % 2 != 0) {
      return {};
    }
    encodedSize += chunk.size();
  }
  result.reserve(encodedSize / 2);

  for (const StringView chunk : hexChunks) {
    for (size_t i = 0; i < chunk.size(); i += 2) {
      const int high = hexDigit(chunk[i]);
      const int low = hexDigit(chunk[i + 1]);
      if (high < 0 || low < 0) {
        return {};
      }
      result.push_back(static_cast<char>((high << 4) | low));
    }
  }
  return result;
}

void appendEscapedHtml(std::string &output, StringView text) {
  for (const char value : text) {
    switch (value) {
    case '&':
      output += "&amp;";
      break;
    case '<':
      output += "&lt;";
      break;
    case '>':
      output += "&gt;";
      break;
    case '"':
      output += "&quot;";
      break;
    case '\'':
      output += "&#39;";
      break;
    default:
      output.push_back(value);
      break;
    }
  }
}

std::string generateHtml() {
  std::string html;
  html.reserve(128 * 1024);
  html += R"(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sinen licenses</title>
<style>
:root { color-scheme: light dark; font-family: system-ui, sans-serif; }
body { max-width: 960px; margin: 0 auto; padding: 2rem; line-height: 1.5; }
h1 { margin-bottom: .25rem; }
.summary { color: #777; margin-top: 0; }
nav { columns: 2 18rem; margin: 2rem 0; }
section { border-top: 1px solid #8886; padding: 1.5rem 0; }
.meta { font-size: .9rem; color: #777; }
pre { padding: 1rem; overflow: auto; white-space: pre-wrap; border-radius: .5rem;
      background: #8881; font: .85rem/1.45 ui-monospace, monospace; }
a { color: #268bd2; }
</style>
</head>
<body>
<h1>Sinen licenses</h1>
<p class="summary">Generated automatically by Sinen at startup.</p>
<nav><ol>
)";

  size_t index = 0;
  for (const auto &entry : license_data::entries) {
    html += "<li><a href=\"#license-";
    html += std::to_string(index);
    html += "\">";
    appendEscapedHtml(html, entry.name);
    html += "</a></li>\n";
    ++index;
  }
  html += "</ol></nav>\n";

  index = 0;
  for (const auto &entry : license_data::entries) {
    html += "<section id=\"license-";
    html += std::to_string(index);
    html += "\"><h2>";
    appendEscapedHtml(html, entry.name);
    html += "</h2><p class=\"meta\">";
    if (!entry.url.empty()) {
      html += "<a href=\"";
      appendEscapedHtml(html, entry.url);
      html += "\">Project website</a>";
    }
    if (!entry.source.empty()) {
      html += " &middot; Source: <code>";
      appendEscapedHtml(html, entry.source);
      html += "</code>";
    }
    html += "</p>\n";
    if (!entry.note.empty()) {
      html += "<p>";
      appendEscapedHtml(html, entry.note);
      html += "</p>\n";
    }

    const std::string licenseText = decodeHex(entry.textHexChunks);
    if (!licenseText.empty()) {
      html += "<pre>";
      appendEscapedHtml(html, licenseText);
      html += "</pre>\n";
    }
    html += "</section>\n";
    ++index;
  }

  html += "</body>\n</html>\n";
  return html;
}
} // namespace

bool LicenseReport::writeHtml(StringView outputPath) {
  const std::filesystem::path path{std::string(outputPath)};
  std::error_code error;
  if (path.has_parent_path()) {
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
      return false;
    }
  }

  const std::string html = generateHtml();
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) {
    return false;
  }
  output.write(html.data(), static_cast<std::streamsize>(html.size()));
  return output.good();
}
} // namespace sinen
