#ifndef VTEXTDOCUMENTLAYOUT_H
#define VTEXTDOCUMENTLAYOUT_H

#include <QAbstractTextDocumentLayout>
#include <QVector>

class VTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT

public:
    explicit VTextDocumentLayout(QTextDocument *p_doc);

    void draw(QPainter *p_painter, const PaintContext &p_context) Q_DECL_OVERRIDE;

    int hitTest(const QPointF &p_point, Qt::HitTestAccuracy p_accuracy) const Q_DECL_OVERRIDE;

    int pageCount() const Q_DECL_OVERRIDE;

    QSizeF documentSize() const Q_DECL_OVERRIDE;

    QRectF frameBoundingRect(QTextFrame *p_frame) const Q_DECL_OVERRIDE;

    QRectF blockBoundingRect(const QTextBlock &p_block) const Q_DECL_OVERRIDE;

    void setCursorWidth(int p_width);

    int cursorWidth() const;

protected:
    void documentChanged(int p_from, int p_oldLength, int p_length) Q_DECL_OVERRIDE;

private:
    struct BlockInfo
    {
        BlockInfo()
            : m_valid(true),
              m_offset(-1),
              m_rect(QRectF())
        {
        }

        void reset()
        {
            m_valid = true;
            m_offset = -1;
            m_rect = QRectF();
        }

        // Whether this block is valid and visible.
        bool m_valid;

        // The offset Y of this block.
        // -1 for invalid.
        qreal m_offset;

        // The bounding rect of the content.
        // Including the right/bottom margin.
        // Null for invalid.
        QRectF m_rect;
    };

    void layoutBlock(const QTextBlock &p_block);

    // Returns the content width of the longest line within @p_block.
    qreal blockWidth(const QTextBlock &p_block);

    // Clear the layout of @p_block.
    // Also clear all the offset behind this block.
    void clearBlockLayout(QTextBlock &p_block);

    // Clear the offset of all the blocks from @p_blockNumber.
    void clearOffsetFrom(int p_blockNumber);

    // Fill the offset filed from @p_blockNumber + 1.
    void fillOffsetFrom(int p_blockNumber);

    void updateBlockCount(int p_count);

    bool validateBlocks() const;

    // Set the width of the page.
    void setPageWidth(int p_width);

    void finishBlockLayout(const QTextBlock &p_block);

    // Find next block that is valid and visible.
    int nextValidBlockNumber(int p_number) const;

    // Find previous block that is valid and visible.
    // @p_number block has valid offset.
    int previousValidBlockNumber(int p_number) const;

    qreal blockWidth(const QTextBlock &p_block) const;

    // Update block count and m_blocks size.
    void updateDocumentSize();

    QVector<QTextLayout::FormatRange> formatRangeFromSelection(const QTextBlock &p_block,
                                                               const QVector<Selection> &p_selections) const;

    // Get the block range [first, last] by rect @p_rect.
    // @p_rect: a clip region in document coordinates. If null, returns all the blocks.
    // Return [-1, -1] if no valid block range found.
    void blockRangeFromRect(const QRectF &p_rect, int &p_first, int &p_last) const;

    // Available width of the page.
    qreal m_pageWidth;

    // Document margin on left/right/bottom.
    qreal m_margin;

    // Maximum width of the contents.
    qreal m_width;

    // The block number of the block which contains the m_width.
    int m_maximumWidthBlockNumber;

    // Height of all the document (all the blocks).
    qreal m_height;

    // Block count of the document.
    int m_blockCount;

    // Width of the cursor.
    int m_cursorWidth;

    QVector<BlockInfo> m_blocks;
};

inline void VTextDocumentLayout::setPageWidth(int p_width)
{
    m_pageWidth = m_width = p_width;
}

#endif // VTEXTDOCUMENTLAYOUT_H
