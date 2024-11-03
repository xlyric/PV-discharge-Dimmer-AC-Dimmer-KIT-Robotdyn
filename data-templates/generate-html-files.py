#!/usr/bin/python
"""Generate HTML files from Mako template files"""

import re
import os

from mako.lookup import TemplateLookup

data_templates_dir = os.path.dirname(os.path.realpath(__file__))
templates_dir = os.path.join(data_templates_dir, "templates")
output_dir = os.path.realpath(os.path.join(data_templates_dir, "..", "data"))

lookup = TemplateLookup(directories=[templates_dir])

print("Generate HTML files:")
for file in os.listdir(templates_dir):
    # Only handle .tpl files
    filename = os.fsdecode(file)
    if not filename.endswith(".tpl"):
        continue

    # Compute output file path
    output_filepath = os.path.join(output_dir, filename.replace(".tpl", ".html"))
    print(f"- {os.path.basename(output_filepath)}...", end=" ")

    # Generate HTML content from Mako template file
    tpl = lookup.get_template(file)
    content = tpl.render()

    # Remove trailing spaces added by Mako templating
    content = re.sub(r"\s+$", "", content, flags=re.MULTILINE)

    # Add newline at end of file
    content += "\n"

    # Write content in output file
    with open(output_filepath, "w", encoding="utf8") as fd:
        fd.write(content)

    print("done.")
print("done.")
