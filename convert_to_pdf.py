import markdown
import pdfkit
import base64
import os
import re
from pathlib import Path

path_wkhtmltopdf = r"C:\Program Files\wkhtmltopdf\bin\wkhtmltopdf.exe"
config = pdfkit.configuration(wkhtmltopdf=path_wkhtmltopdf)

def embed_images(md_text, base_path):
    def repl(match):
        img_path = os.path.join(base_path, match.group(1))
        if os.path.exists(img_path):
            with open(img_path, "rb") as img_file:
                b64_img = base64.b64encode(img_file.read()).decode()
                ext = os.path.splitext(img_path)[1][1:]  # extension
                return f'<img src="data:image/{ext};base64,{b64_img}">'
        return match.group(0)
    return re.sub(r'!\[.*?\]\((.*?)\)', repl, md_text)

def convert_md_to_pdf():
    md_file = Path("day2.md")
    base_path = str(md_file.parent.resolve())

    # Read + embed images
    with open(md_file, "r", encoding="utf-8") as f:
        md_content = f.read()
    md_content = embed_images(md_content, base_path)

    # Markdown → HTML
    html_content = markdown.markdown(md_content, extensions=["extra", "toc", "tables", "fenced_code"])

    # Force resize for all <img>
    html_content = re.sub(
        r'<img(.*?)>',
        r'<img\1 style="max-width:300px; height:auto; display:block; margin:10px auto;">',
        html_content
    )

    # Full HTML with UTF-8
    html_doc = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8">
        <style>
            body {{ font-family: Arial, sans-serif; margin: 40px; }}
            code {{ background-color: #f4f4f4; padding: 2px 5px; }}
            pre {{ background-color: #f4f4f4; padding: 10px; }}
        </style>
    </head>
    <body>
        {html_content}
    </body>
    </html>
    """

    # Options: utf-8 + local files
    options = {
        "encoding": "utf-8",
        "enable-local-file-access": None
    }

    output_file = md_file.with_suffix(".pdf")
    pdfkit.from_string(html_doc, str(output_file), configuration=config, options=options)
    print(f"✅ PDF created successfully: {output_file}")

if __name__ == "__main__":
    convert_md_to_pdf()
