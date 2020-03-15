using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class hpController : MonoBehaviour {

    public Transform owner;
    public bool setFlag = false;
    public Vector3 offset = new Vector3(0.05f,0.55f,0f);

    private Slider hpSlider;
    private int maxHP, currentHP;

    // Update is called once per frame
    void Update () {
        if (!setFlag) return;
        hpSlider.transform.position = owner.position + offset;
        hpSlider.transform.rotation = Camera.main.transform.rotation;
    }

    public void setController(GameObject _owner, int _maxHP, int _currentHP, Slider _hpSlider)
    {
        owner = _owner.transform.Find("HeadTarget");
        maxHP = _maxHP;
        currentHP = _currentHP;
        hpSlider = _hpSlider;
        setFlag = true;
    }

    public void setHP(int _currentHP)
    {
        if (_currentHP == 0)
            hpSlider.value = 0;
        else
            hpSlider.value = (float)_currentHP / (float)maxHP;
    }
}
