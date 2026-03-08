# セッション反省会議事録

日時: 2026-03-07
ブランチ: master
セッションスコープ: PPTX 自動生成基盤の構築（md2pptx + EDIA ロードマップ再現）

---

## 1. セッション成果サマリ

| # | 成果物 | 内容 |
|---|--------|------|
| 1 | `scripts/md2pptx.py` | Markdown → PowerPoint 自動変換ツール（8レイアウト×5テーマ） |
| 2 | `scripts/gen_edia_onepage.py` | EDIA ロードマップ 1枚スライド生成（元スライド忠実再現） |
| 3 | `docs/samples/EDIA_roadmap_sample.md` | md2pptx 用サンプル Markdown |
| 4 | `.claude/skills/md2pptx/SKILL.md` | md2pptx スキル定義 |
| 5 | `.claude/knowledge/md2pptx_guide.md` | md2pptx 詳細ガイド |
| 6 | `.claude/knowledge/pptx_advanced_shapes.md` | §14 追記（swooshArrow/gradFill/wedgeRectCallout） |

## 2. 発生した問題

| # | 問題 | 影響 | 解決 |
|---|------|------|------|
| 1 | swooshArrow が見えない | テーマ色 schemeClr が default Presentation の dk2=#1F497D にマップされ、alpha=50000 で白背景に溶けた | srgbClr に変更 + alpha=75000→80000 に調整 |
| 2 | swooshArrow の位置はみ出し（左右） | left が負値、left+width がスライド幅超過を交互に繰り返した | left≥0 かつ left+width≤13.333 の制約を明確化 |
| 3 | 吹き出し形状の不一致 | wedgeRoundRectCallout（角丸）で作ったが元は wedgeRectCallout（角なし） | 元スライドの XML を分析して正確な形状・adj 値を特定 |
| 4 | グラデーション方向が逆 | ang=0（左→右）で実装したが、元は右→左 | ang=10800000（180°）に修正 |
| 5 | 座標の手動調整ループ | 5回以上の「生成→確認→微調整」サイクル | md2pptx の自動レイアウト計算で根本解決 |

## 3. 根本原因分析

- **問題1-4**: 元スライドの XML を最初に徹底分析しなかった。schemeClr vs srgbClr の違い、swooshArrow の正確な座標・回転・adj 値を先に全抽出すべきだった
- **問題5**: ピクセル単位の座標をハードコードする設計方針が非効率。Phase 数から自動計算するアルゴリズムに切り替えて解決

## 4. 良かった点

- swoosh_test.pptx で図形単体テストを行い、描画自体は正常と確認できた（問題の切り分け）
- ユーザーが PowerPoint で手動調整した差分を抽出→スクリプトに反映するワークフローが機能
- 最終的に md2pptx.py として汎用化し、ハードコード座標問題を根本解決

## 5. 改善すべき点

| # | 問題 | 対策 | 状態 |
|---|------|------|------|
| 1 | 元スライドの XML 分析が不十分なまま実装開始 | PPTX 再現時は最初に全図形の XML ダンプを実行する手順を確立 | 未 |
| 2 | schemeClr/srgbClr の違いを見落とし | pptx_advanced_shapes.md に注意事項追記済み（§14） | 済 |
| 3 | 座標調整の試行錯誤ループ | md2pptx の自動レイアウト計算で解決。新規作成時はハードコード座標を避ける | 済 |

## 6. 教訓

- **PPTX 再現はまず XML 分析**: 見た目のコピーではなく、元ファイルの XML を正確に抽出してから実装する
- **schemeClr は罠**: `Presentation()` のデフォルトテーマと元スライドのテーマが異なると色が変わる。常に srgbClr を使う
- **alpha 値の直感との乖離**: 50000(50%) は白背景では薄すぎる。80000(80%) が実用的な最低ライン
- **座標ハードコードは技術的負債**: Phase 数やコンテンツ量に応じた自動計算にすべき

## 7. 前回反省会フォローアップ

前回（S22）の改善項目:
- PDCA §2.0 運用定着 → 今回は PPTX 作成タスクで PDCA 構造は明示的に適用せず（創作的タスクのため）
- packages/ 廃止 → 影響なし（今回の作業範囲外）
