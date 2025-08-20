import markdown
import pdfkit
from pathlib import Path

def convert_md_to_pdf():
    # Read the markdown file
    md_file = Path('day2.md')
    with open(md_file, 'r', encoding='utf-8') as f:
        md_content = f.read()

    # Convert markdown to HTML
    html_content = markdown.markdown(
        md_content,
        extensions=['extra', 'toc', 'tables', 'fenced_code']
    )

    # Add CSS for better formatting
    html_doc = f'''
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <style>
            body {{ font-family: Arial, sans-serif; margin: 40px; }}
            code {{ background-color: #f4f4f4; padding: 2px 5px; }}
            pre {{ background-color: #f4f4f4; padding: 10px; }}
            img {{ max-width: 100%; }}
        </style>
    </head>
    <body>
        {html_content}
    </body>
    </html>
    '''

    # Convert HTML to PDF
    output_file = md_file.with_suffix('.pdf')
    pdfkit.from_string(html_doc, str(output_file))
    print(f"PDF created successfully: {output_file}")

if __name__ == '__main__':
    convert_md_to_pdf()
