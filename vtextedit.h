#ifndef VTEXTEDIT_H
#define VTEXTEDIT_H

#include <QTextEdit>
#include <QTextBlock>

#include "vlinenumberarea.h"

class VTextDocumentLayout;
class QPainter;
class QResizeEvent;
class VImageResourceManager2;


struct VBlockImageInfo2
{
public:
    VBlockImageInfo2()
        : m_blockNumber(-1),
          m_startPos(-1),
          m_endPos(-1),
          m_padding(0),
          m_inlineImage(false)
    {
    }

    VBlockImageInfo2(int p_blockNumber,
                     const QString &p_imageName,
                     int p_startPos = -1,
                     int p_endPos = -1,
                     int p_padding = 0,
                     bool p_inlineImage = false)
        : m_blockNumber(p_blockNumber),
          m_startPos(p_startPos),
          m_endPos(p_endPos),
          m_padding(p_padding),
          m_inlineImage(p_inlineImage),
          m_imageName(p_imageName)
    {
    }

    // Block number.
    int m_blockNumber;

    // Start position of the image link in block.
    int m_startPos;

    // End position of the image link in block.
    int m_endPos;

    // Padding of the image.
    int m_padding;

    // Whether it is inline image or block image.
    bool m_inlineImage;

    // The name of the image corresponding to this block.
    QString m_imageName;

private:
    // For cache only.
    QSize m_imageSize;

    friend class VImageResourceManager2;
    friend class VTextDocumentLayout;
};


class VTextEdit : public QTextEdit, public VTextEditWithLineNumber
{
    Q_OBJECT
public:
    explicit VTextEdit(QWidget *p_parent = nullptr);

    explicit VTextEdit(const QString &p_text, QWidget *p_parent = nullptr);

    virtual ~VTextEdit();

    void init();

    void setLineLeading(qreal p_leading);

    void paintLineNumberArea(QPaintEvent *p_event) Q_DECL_OVERRIDE;

    void setLineNumberType(LineNumberType p_type);

    void setLineNumberColor(const QColor &p_foreground, const QColor &p_background);

    QTextBlock firstVisibleBlock() const;

    // Update images of these given blocks.
    // Images of blocks not given here will be clear.
    void updateBlockImages(const QVector<VBlockImageInfo2> &p_blocksInfo);

    void clearBlockImages();

    // Whether the resoruce manager contains image of name @p_imageName.
    bool containsImage(const QString &p_imageName) const;

    // Add an image to the resources.
    void addImage(const QString &p_imageName, const QPixmap &p_image);

    void setBlockImageEnabled(bool p_enabled);

    void setImageWidthConstrainted(bool p_enabled);

protected:
    void resizeEvent(QResizeEvent *p_event) Q_DECL_OVERRIDE;

private slots:
    // Update viewport margin to hold the line number area.
    void updateLineNumberAreaMargin();

    void updateLineNumberArea();

private:
    VTextDocumentLayout *getLayout() const;

    // Return the Y offset of the content via the scrollbar.
    int contentOffsetY() const;

    VLineNumberArea *m_lineNumberArea;

    LineNumberType m_lineNumberType;

    VImageResourceManager2 *m_imageMgr;

    bool m_blockImageEnabled;
};

inline void VTextEdit::setLineNumberType(LineNumberType p_type)
{
    if (p_type == m_lineNumberType) {
        return;
    }

    m_lineNumberType = p_type;

    updateLineNumberArea();
}

inline void VTextEdit::setLineNumberColor(const QColor &p_foreground,
                                          const QColor &p_background)
{
    m_lineNumberArea->setForegroundColor(p_foreground);
    m_lineNumberArea->setBackgroundColor(p_background);
}

#endif // VTEXTEDIT_H
